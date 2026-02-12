#include "SWCDetection/LoopDetection.h"
#include "SWCDetection/swc_utils.h"
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

namespace swcdetection {

std::vector<DetectionResult> loopDetection(
    V_NeuronSWC_list& inputSegList,
    double dist_thresh)
{
    std::vector<DetectionResult> results;

    std::map<std::string, std::set<size_t>> wholeGrid2segIDmap;
    std::map<std::string, bool> isEndPointMap;
    std::map<std::string, std::set<std::string>> parentMap;
    std::set<std::string> allPoints;
    std::map<std::string, std::set<std::string>> childMap;

    for (size_t i = 0; i < inputSegList.seg.size(); ++i) {
        V_NeuronSWC seg = inputSegList.seg[i];
        std::vector<int> rowN2Index(seg.row.size() + 1);

        for (size_t j = 0; j < seg.row.size(); ++j) {
            rowN2Index[(int)seg.row[j].n] = j;
        }

        for (size_t j = 0; j < seg.row.size(); ++j) {
            std::string gridKey = makeGridKey(seg.row[j].x, seg.row[j].y, seg.row[j].z);
            wholeGrid2segIDmap[gridKey].insert(i);
            allPoints.insert(gridKey);

            if (seg.row[j].parent != -1) {
                int parentIdx = rowN2Index[(int)seg.row[j].parent];
                std::string parentKey = makeGridKey(seg.row[parentIdx].x, seg.row[parentIdx].y, seg.row[parentIdx].z);
                parentMap[gridKey].insert(parentKey);
                childMap[parentKey].insert(gridKey);
            }

            if (j == 0 || j == seg.row.size() - 1) {
                isEndPointMap[gridKey] = true;
            }
        }
    }

    // Build endpoint/branching-point graph
    std::vector<std::string> points;
    std::vector<std::set<int>> linksIndex;
    std::map<std::string, int> pointsIndexMap;

    for (size_t i = 0; i < inputSegList.seg.size(); ++i) {
        V_NeuronSWC seg = inputSegList.seg[i];
        for (size_t j = 0; j < seg.row.size(); ++j) {
            std::string gridKey = makeGridKey(seg.row[j].x, seg.row[j].y, seg.row[j].z);
            if (j == 0 || j == seg.row.size() - 1) {
                if (pointsIndexMap.find(gridKey) == pointsIndexMap.end()) {
                    points.push_back(gridKey);
                    linksIndex.push_back(std::set<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            } else {
                if (wholeGrid2segIDmap[gridKey].size() > 1 &&
                    isEndPointMap.find(gridKey) != isEndPointMap.end() &&
                    pointsIndexMap.find(gridKey) == pointsIndexMap.end()) {
                    points.push_back(gridKey);
                    linksIndex.push_back(std::set<int>());
                    pointsIndexMap[gridKey] = points.size() - 1;
                }
            }
        }
    }

    for (size_t i = 0; i < inputSegList.seg.size(); ++i) {
        V_NeuronSWC seg = inputSegList.seg[i];
        std::vector<int> segIndexs;
        std::set<int> segIndexsSet;
        for (size_t j = 0; j < seg.row.size(); ++j) {
            std::string gridKey = makeGridKey(seg.row[j].x, seg.row[j].y, seg.row[j].z);
            if (pointsIndexMap.find(gridKey) != pointsIndexMap.end()) {
                int index = pointsIndexMap[gridKey];
                if (segIndexsSet.find(index) == segIndexsSet.end()) {
                    segIndexs.push_back(index);
                    segIndexsSet.insert(index);
                }
            }
        }
        for (size_t j = 0; j + 1 < segIndexs.size(); ++j) {
            linksIndex[segIndexs[j]].insert(segIndexs[j + 1]);
            linksIndex[segIndexs[j + 1]].insert(segIndexs[j]);
        }
    }

    // Prune leaf nodes iteratively to find cycles
    bool isDeleteEnd = false;
    while (!isDeleteEnd) {
        isDeleteEnd = true;
        for (int i = 0; i < (int)points.size(); ++i) {
            if (linksIndex[i].size() == 1) {
                int linkIndex = *(linksIndex[i].begin());
                linksIndex[i].clear();
                linksIndex[linkIndex].erase(std::find(linksIndex[linkIndex].begin(), linksIndex[linkIndex].end(), i));
                isDeleteEnd = false;
            }
        }
    }

    // Detect loops with 3+ edges
    std::vector<std::string> newpoints;
    for (size_t i = 0; i < points.size(); ++i) {
        if (linksIndex[i].size() >= 2)
            newpoints.push_back(points[i]);
    }

    std::set<std::string> specPoints;
    for (size_t i = 0; i < newpoints.size(); ++i) {
        if (parentMap[newpoints[i]].size() >= 2) {
            specPoints.insert(newpoints[i]);
        }
    }

    // Detect 2-edge loops
    for (size_t i = 0; i < inputSegList.seg.size(); ++i) {
        V_NeuronSWC seg = inputSegList.seg[i];
        std::string gridKey1 = makeGridKey(seg.row[0].x, seg.row[0].y, seg.row[0].z);
        std::string gridKey2 = makeGridKey(seg.row[seg.row.size() - 1].x, seg.row[seg.row.size() - 1].y, seg.row[seg.row.size() - 1].z);
        if (gridKey1 == gridKey2) {
            continue;
        }
        std::set<size_t> segSet1 = wholeGrid2segIDmap[gridKey1];
        std::set<size_t> segSet2 = wholeGrid2segIDmap[gridKey2];
        std::set<size_t> intersectionSet;
        std::set_intersection(segSet1.begin(), segSet1.end(), segSet2.begin(), segSet2.end(),
                              std::inserter(intersectionSet, intersectionSet.begin()));

        if (intersectionSet.size() >= 2) {
            std::vector<size_t> maybeOverlapSegIdVec;
            std::set<size_t> overlapSegIdSet;
            for (auto it = intersectionSet.begin(); it != intersectionSet.end(); it++) {
                maybeOverlapSegIdVec.push_back(*it);
            }
            for (size_t m = 0; m < maybeOverlapSegIdVec.size(); m++) {
                for (size_t n = m + 1; n < maybeOverlapSegIdVec.size(); n++) {
                    int result = isOverlapOfTwoSegs(inputSegList.seg[maybeOverlapSegIdVec[m]], inputSegList.seg[maybeOverlapSegIdVec[n]]);
                    if (result == 1)
                        overlapSegIdSet.insert(maybeOverlapSegIdVec[m]);
                    if (result == 2)
                        overlapSegIdSet.insert(maybeOverlapSegIdVec[n]);
                }
            }

            if (maybeOverlapSegIdVec.size() - overlapSegIdSet.size() != 1) {
                specPoints.insert(gridKey1);
                specPoints.insert(gridKey2);
            }

            // Mark overlapping segments for deletion (detection only, no server-side deletion)
            for (auto it = overlapSegIdSet.begin(); it != overlapSegIdSet.end(); it++) {
                inputSegList.seg[*it].to_be_deleted = true;
            }
        }
    }

    // Detect loops from intersecting segments
    for (auto it = wholeGrid2segIDmap.begin(); it != wholeGrid2segIDmap.end(); it++) {
        if (it->second.size() == 2) {
            bool isLoopExists = true;
            std::string coor = it->first;
            for (auto segIt = it->second.begin(); segIt != it->second.end(); segIt++) {
                V_NeuronSWC seg = inputSegList.seg[*segIt];
                std::string gk1 = makeGridKey(seg.row[0].x, seg.row[0].y, seg.row[0].z);
                std::string gk2 = makeGridKey(seg.row[seg.row.size() - 1].x, seg.row[seg.row.size() - 1].y, seg.row[seg.row.size() - 1].z);
                if (coor == gk1 || coor == gk2) {
                    isLoopExists = false;
                }
            }
            if (isLoopExists) {
                specPoints.insert(coor);
            }
        }
    }

    // Convert specPoints to results
    for (auto it = specPoints.begin(); it != specPoints.end(); it++) {
        float x, y, z;
        stringToXYZ(*it, x, y, z);
        results.push_back({x, y, z, 0, "Loop"});
    }

    return results;
}

} // namespace swcdetection
