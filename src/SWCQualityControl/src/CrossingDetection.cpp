#include "SWCDetection/CrossingDetection.h"
#include "SWCDetection/swc_utils.h"

// Include octree and crossing detect infrastructure
#include "octree.h"
#include "octree_container.h"

#include <unordered_set>
#include <unordered_map>
#include <future>
#include <mutex>
#include <iostream>
#include <queue>
#include <chrono>
#include <thread>

// ---- Octree adaptor types for crossing detection ----
namespace crossing_util {
    using namespace OrthoTree;
    using BoundingBoxNode = std::array<util::Node, 2>;

    struct AdaptorBasicsCustom {
        static inline float& point_comp(util::Node& pt, OrthoTree::dim_type iDimension) {
            switch (iDimension) {
                case 0: return pt.x;
                case 1: return pt.y;
                case 2: return pt.z;
                default: assert(false); return pt.x;
            }
        }
        static constexpr float point_comp_c(util::Node const& pt, OrthoTree::dim_type iDimension) {
            switch (iDimension) {
                case 0: return pt.x;
                case 1: return pt.y;
                case 2: return pt.z;
                default: assert(false); return pt.x;
            }
        }
        static inline util::Node& box_min(BoundingBoxNode& box) { return box[0]; }
        static inline util::Node& box_max(BoundingBoxNode& box) { return box[1]; }
        static constexpr util::Node const& box_min_c(BoundingBoxNode const& box) { return box[0]; }
        static constexpr util::Node const& box_max_c(BoundingBoxNode const& box) { return box[1]; }
    };

    using AdaptorCustom = OrthoTree::AdaptorGeneralBase<3, util::Node, BoundingBoxNode, AdaptorBasicsCustom, float>;
    using OctreePointCustom = OrthoTree::OrthoTreePoint<3, util::Node, BoundingBoxNode, AdaptorCustom, float>;

    inline bool boundingBoxIntersect(const BoundingBoxNode& b1, const BoundingBoxNode& b2) {
        return (b1[0].x <= b2[1].x && b1[1].x >= b2[0].x) &&
               (b1[0].y <= b2[1].y && b1[1].y >= b2[0].y) &&
               (b1[0].z <= b2[1].z && b1[1].z >= b2[0].z);
    }

    using BranchesType = std::vector<std::pair<std::vector<util::Node>, BoundingBoxNode>>;
    using KeyPointsType = std::vector<
        std::pair<
            std::pair<
                std::pair<util::Node, int>,
                std::pair<util::Node, int>
            >,
            bool>>;

    inline void findBranches(int root, const std::unordered_map<int, std::vector<int>>& childMap,
                              std::unordered_map<int, util::Node>& nodeMap,
                              BranchesType& branches,
                              std::unordered_set<util::Node>& branchingNodes) {
        auto updateBoundingBox = [](util::Node& ref, std::pair<std::vector<util::Node>, BoundingBoxNode>& currentBranch) {
            if (ref.x < currentBranch.second[0].x) currentBranch.second[0].x = ref.x;
            if (ref.y < currentBranch.second[0].y) currentBranch.second[0].y = ref.y;
            if (ref.z < currentBranch.second[0].z) currentBranch.second[0].z = ref.z;
            if (ref.x > currentBranch.second[1].x) currentBranch.second[1].x = ref.x;
            if (ref.y > currentBranch.second[1].y) currentBranch.second[1].y = ref.y;
            if (ref.z > currentBranch.second[1].z) currentBranch.second[1].z = ref.z;
        };

        std::queue<std::pair<int, std::pair<std::vector<util::Node>, BoundingBoxNode>>> q;
        q.push({root, {std::vector<util::Node>{},
                       BoundingBoxNode{
                           util::Node{1, -1, std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()},
                           util::Node{1, -1, -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()}
                       }}});

        while (!q.empty()) {
            auto [nodeId, currentBranch] = q.front();
            q.pop();
            auto it = childMap.find(nodeId);
            if (it == childMap.end() || it->second.size() != 1) {
                if (!currentBranch.first.empty()) {
                    util::Node lowBounding{1, -1};
                    lowBounding.x = currentBranch.second[0].x - 6 * 1.7f;
                    lowBounding.y = currentBranch.second[0].y - 6 * 1.7f;
                    lowBounding.z = currentBranch.second[0].z - 6 * 1.7f;
                    util::Node highBounding{1, -1};
                    highBounding.x = currentBranch.second[1].x + 6 * 1.7f;
                    highBounding.y = currentBranch.second[1].y + 6 * 1.7f;
                    highBounding.z = currentBranch.second[1].z + 6 * 1.7f;
                    updateBoundingBox(lowBounding, currentBranch);
                    updateBoundingBox(highBounding, currentBranch);
                    branches.push_back(currentBranch);
                }
                if (it != childMap.end()) {
                    auto nodeInfo = nodeMap[nodeId];
                    branchingNodes.insert(nodeInfo);
                    for (int childId : it->second) {
                        q.push({childId, {std::vector{nodeInfo}, BoundingBoxNode{
                            util::Node{1, -1, nodeInfo.x, nodeInfo.y, nodeInfo.z},
                            util::Node{1, -1, nodeInfo.x, nodeInfo.y, nodeInfo.z}
                        }}});
                    }
                }
            } else {
                auto ref = nodeMap[nodeId];
                currentBranch.first.push_back(ref);
                updateBoundingBox(ref, currentBranch);
                q.push({it->second[0], currentBranch});
            }
        }
    }

    // Config constants (matching ConfigManager defaults)
    struct Config {
        size_t ignoreShortBranchSize = 12;
        float nearestPointDistanceThreshold = 2.0f;
        size_t nearestPointParentNodeNumThreshold = 10;
        size_t nearestPointChildNodeNumThreshold = 10;
        float forkPointIgnoreDistanceThreshold = 5.0f * 1.17f;
        float mergeNearestPointOnSameBranchThreshold = 20.0f;
        float edgePointIgnoreThreshold = 33.0f;
    };
}

namespace swcdetection {

std::vector<CrossingResult> crossingDetection(
    V_NeuronSWC_list& inputSegList,
    const XYZ* imageMaxRes)
{
    std::vector<CrossingResult> results;

    // Convert V_NeuronSWC_list to flat node list
    // We need to renumber nodes sequentially, building parent-child relationships per segment
    std::vector<util::Node> allNodes;
    int nodeCounter = 1;

    for (size_t segIdx = 0; segIdx < inputSegList.seg.size(); ++segIdx) {
        V_NeuronSWC& seg = inputSegList.seg[segIdx];
        std::map<int, int> oldToNewN;

        for (size_t j = 0; j < seg.row.size(); ++j) {
            int oldN = (int)seg.row[j].n;
            oldToNewN[oldN] = nodeCounter;
            nodeCounter++;
        }

        for (size_t j = 0; j < seg.row.size(); ++j) {
            util::Node node;
            node.n = oldToNewN[(int)seg.row[j].n];
            node.x = seg.row[j].x;
            node.y = seg.row[j].y;
            node.z = seg.row[j].z;
            int oldParent = (int)seg.row[j].parent;
            if (oldParent == -1 || oldToNewN.find(oldParent) == oldToNewN.end()) {
                node.parent = -1;
            } else {
                node.parent = oldToNewN[oldParent];
            }
            allNodes.push_back(node);
        }
    }

    if (allNodes.empty())
        return results;

    // Process nodes in chunks per connected component (root node = parent==-1)
    crossing_util::Config config;

    std::vector<util::Node> nodes;
    std::vector<int> rootNodeIds;

    crossing_util::KeyPointsType allKeyPoints;

    int curNode = 0;
    for (auto& neuron : allNodes) {
        nodes.push_back(neuron);
        if (neuron.parent == -1) {
            rootNodeIds.push_back(neuron.n);
        }
        curNode++;

        if ((rootNodeIds.size() == 2) || (rootNodeIds.size() == 1 && (size_t)curNode == allNodes.size())) {
            util::Node lastNode{};
            int lastRootNodeId = -1;
            bool multiRoot = false;
            if (rootNodeIds.size() == 2) {
                multiRoot = true;
            }

            if (multiRoot) {
                lastNode = nodes.back();
                lastRootNodeId = rootNodeIds.back();
                nodes.pop_back();
                rootNodeIds.pop_back();
            }

            if (rootNodeIds.size() != 1) {
                nodes.clear();
                rootNodeIds.clear();
                if (multiRoot) {
                    nodes.push_back(lastNode);
                    rootNodeIds.push_back(lastRootNodeId);
                }
                continue;
            }

            // Build node map and child map
            std::unordered_map<int, util::Node> nodeMap;
            for (auto& n : nodes) {
                nodeMap[n.n] = n;
            }
            auto childNodeMap = util::buildChildMap(nodes);

            // Generate branches
            crossing_util::BranchesType branches;
            std::unordered_set<util::Node> branchingNodes;
            crossing_util::findBranches(rootNodeIds[0], childNodeMap, nodeMap, branches, branchingNodes);

            // Select branches (filter short ones)
            crossing_util::BranchesType selectedBranches;
            for (const auto& branch : branches) {
                if (branch.first.size() > config.ignoreShortBranchSize) {
                    selectedBranches.push_back(branch);
                }
            }

            // Generate nearest key points
            crossing_util::KeyPointsType keyPoints;
            auto maxThreads = std::thread::hardware_concurrency();
            std::mutex mutex1;
            std::vector<std::future<void>> futures1;

            for (int cidx1 = 0; cidx1 < (int)selectedBranches.size(); cidx1++) {
                auto task = [&selectedBranches, &keyPoints, &mutex1, &config](int cidx1) {
                    crossing_util::KeyPointsType currentKeyPoints;

                    for (int cidx2 = cidx1 + 1; cidx2 < (int)selectedBranches.size(); cidx2++) {
                        if (crossing_util::boundingBoxIntersect(selectedBranches[cidx1].second, selectedBranches[cidx2].second)) {
                            std::span<util::Node> span2 = selectedBranches[cidx2].first;
                            crossing_util::OctreePointCustom octree1(span2, 8);

                            for (int bidx1 = 0; bidx1 < (int)selectedBranches[cidx1].first.size(); bidx1++) {
                                auto nearstPointVec = octree1.GetNearestNeighbors(selectedBranches[cidx1].first[bidx1], 1, span2);
                                if (nearstPointVec.empty()) continue;
                                auto a1 = selectedBranches[cidx1].first[bidx1];
                                auto a2 = span2[nearstPointVec[0]];
                                auto dist = util::neuronDistance(a1, a2);
                                if (dist < config.nearestPointDistanceThreshold
                                    && selectedBranches[cidx1].first.size() > 1
                                    && selectedBranches[cidx2].first.size() > 1) {
                                    bool bAdd = true;
                                    for (auto& ep : currentKeyPoints) {
                                        if (util::neuronDistance(ep.first.second.first, a2) < config.mergeNearestPointOnSameBranchThreshold) {
                                            bAdd = false;
                                            break;
                                        }
                                    }
                                    if (bAdd) {
                                        currentKeyPoints.push_back({{{a1, cidx1}, {a2, cidx2}}, true});
                                    }
                                }
                            }
                        }
                    }
                    {
                        std::lock_guard<std::mutex> lock(mutex1);
                        for (auto& kp : currentKeyPoints) {
                            keyPoints.push_back(kp);
                        }
                    }
                };

                if (futures1.size() >= maxThreads) {
                    for (auto& f : futures1) f.get();
                    futures1.clear();
                }
                futures1.push_back(std::async(std::launch::async, task, cidx1));
            }
            for (auto& f : futures1) f.get();
            futures1.clear();

            // Remove key points near branching nodes or with insufficient parent/child nodes
            auto isParentAndChildHasEnoughNode = [&](int p1, int p2, int b1, int b2) {
                int p1ParentNum = 0, p1ChildNum = 0;
                int p2ParentNum = 0, p2ChildNum = 0;
                auto& b1Branch = selectedBranches[b1].first;
                auto& b2Branch = selectedBranches[b2].first;

                for (int i = 0; i < (int)b1Branch.size(); i++) {
                    if (nodeMap[p1].n == b1Branch[i].n) {
                        p1ParentNum = i;
                        p1ChildNum = b1Branch.size() - i - 1;
                    }
                }
                for (int i = 0; i < (int)b2Branch.size(); i++) {
                    if (nodeMap[p2].n == b2Branch[i].n) {
                        p2ParentNum = i;
                        p2ChildNum = b2Branch.size() - i - 1;
                    }
                }
                return (p1ParentNum >= (int)config.nearestPointParentNodeNumThreshold
                     && p2ParentNum >= (int)config.nearestPointParentNodeNumThreshold
                     && p1ChildNum >= (int)config.nearestPointChildNodeNumThreshold
                     && p2ChildNum >= (int)config.nearestPointChildNodeNumThreshold);
            };

            std::mutex mutex2;
            std::vector<std::future<void>> futures2;
            for (auto& sp : branchingNodes) {
                auto task = [&keyPoints, &isParentAndChildHasEnoughNode, &mutex2, &config](const util::Node& sp) {
                    for (auto& point : keyPoints) {
                        auto d1 = util::neuronDistance(sp, point.first.first.first);
                        auto d2 = util::neuronDistance(sp, point.first.second.first);

                        {
                            std::lock_guard<std::mutex> lock(mutex2);
                            if (!point.second) continue;
                        }

                        auto res = isParentAndChildHasEnoughNode(
                            point.first.first.first.n, point.first.second.first.n,
                            point.first.first.second, point.first.second.second);

                        float distThreshold = config.forkPointIgnoreDistanceThreshold;

                        if (d1 < distThreshold || d2 < distThreshold
                            || point.first.first.first.parent == point.first.second.first.n
                            || point.first.second.first.parent == point.first.first.first.n
                            || !res) {
                            std::lock_guard<std::mutex> lock(mutex2);
                            point.second = false;
                        }
                    }
                };

                if (futures2.size() >= maxThreads) {
                    for (auto& f : futures2) f.get();
                    futures2.clear();
                }
                futures2.push_back(std::async(std::launch::async, task, sp));
            }
            for (auto& f : futures2) f.get();
            futures2.clear();

            // Filter by edge threshold
            if (imageMaxRes) {
                auto edgeThreshold = config.edgePointIgnoreThreshold;
                if (imageMaxRes->x > edgeThreshold * 2
                    && imageMaxRes->y > edgeThreshold * 2
                    && imageMaxRes->z > edgeThreshold * 2) {
                    for (auto& point : keyPoints) {
                        if (!point.second) continue;
                        auto isXOK = point.first.first.first.x >= edgeThreshold && point.first.first.first.x <= imageMaxRes->x - edgeThreshold;
                        auto isYOK = point.first.first.first.y >= edgeThreshold && point.first.first.first.y <= imageMaxRes->y - edgeThreshold;
                        auto isZOK = point.first.first.first.z >= edgeThreshold && point.first.first.first.z <= imageMaxRes->z - edgeThreshold;
                        if (!(isXOK && isYOK && isZOK)) {
                            point.second = false;
                        }
                    }
                }
            }

            for (auto& kp : keyPoints) {
                allKeyPoints.push_back(kp);
            }

            nodes.clear();
            rootNodeIds.clear();

            if (multiRoot) {
                nodes.push_back(lastNode);
                rootNodeIds.push_back(lastRootNodeId);
            }
        }
    }

    // Convert key points to CrossingResult
    for (auto& kp : allKeyPoints) {
        if (kp.second) {
            results.push_back({
                kp.first.first.first.x, kp.first.first.first.y, kp.first.first.first.z,
                kp.first.second.first.x, kp.first.second.first.y, kp.first.second.first.z
            });
        }
    }

    return results;
}

} // namespace swcdetection
