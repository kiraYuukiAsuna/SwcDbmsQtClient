#include "SWCDetection/SWCDetection.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Simple ESWC/SWC file reader: reads into V_NeuronSWC_list
static V_NeuronSWC_list readSWCFile(const std::string& filepath) {
    V_NeuronSWC_list result;

    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open file " << filepath << std::endl;
        return result;
    }

    // Read all nodes
    struct RawNode {
        int n, type, parent;
        float x, y, z, r;
        int seg_id = -1;
        int level = -1;
        int mode = 0;
        double timestamp = 0;
        double tfresindex = 0;
    };

    std::vector<RawNode> rawNodes;
    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        RawNode node;
        iss >> node.n >> node.type >> node.x >> node.y >> node.z >> node.r >> node.parent;
        // Try to read ESWC fields
        iss >> node.seg_id >> node.level >> node.mode >> node.timestamp >> node.tfresindex;

        rawNodes.push_back(node);
    }

    if (rawNodes.empty()) return result;

    // Group nodes into segments by finding connected components via parent links
    // Each root (parent==-1) starts a new segment; follow parent chain
    std::map<int, int> nodeIndex; // n -> index in rawNodes
    for (int i = 0; i < (int)rawNodes.size(); i++) {
        nodeIndex[rawNodes[i].n] = i;
    }

    // Build child map
    std::map<int, std::vector<int>> childMap;
    std::vector<int> roots;
    for (auto& node : rawNodes) {
        if (node.parent == -1) {
            roots.push_back(node.n);
        } else {
            childMap[node.parent].push_back(node.n);
        }
    }

    // Build segments by traversing from each root using DFS, splitting at branching points
    std::function<void(int, V_NeuronSWC&)> buildSegment = [&](int nodeId, V_NeuronSWC& currentSeg) {
        auto& rn = rawNodes[nodeIndex[nodeId]];
        V_NeuronSWC_unit unit;
        unit.n = currentSeg.row.size() + 1;
        unit.type = rn.type;
        unit.x = rn.x;
        unit.y = rn.y;
        unit.z = rn.z;
        unit.r = rn.r;
        unit.parent = (currentSeg.row.empty()) ? -1 : (int)currentSeg.row.size();
        unit.creatmode = rn.mode;
        unit.timestamp = rn.timestamp;
        unit.tfresindex = rn.tfresindex;
        currentSeg.row.push_back(unit);

        auto it = childMap.find(nodeId);
        if (it == childMap.end() || it->second.empty()) {
            // Leaf node - segment ends
            return;
        } else if (it->second.size() == 1) {
            // Linear continuation
            buildSegment(it->second[0], currentSeg);
        } else {
            // Branching point - current segment ends here, start new segments for each child
            for (int childId : it->second) {
                V_NeuronSWC newSeg;
                // Start new segment with the branching point
                V_NeuronSWC_unit branchUnit;
                branchUnit.n = 1;
                branchUnit.type = rn.type;
                branchUnit.x = rn.x;
                branchUnit.y = rn.y;
                branchUnit.z = rn.z;
                branchUnit.r = rn.r;
                branchUnit.parent = -1;
                newSeg.row.push_back(branchUnit);
                buildSegment(childId, newSeg);
                result.append(newSeg);
            }
        }
    };

    for (int rootId : roots) {
        V_NeuronSWC seg;
        buildSegment(rootId, seg);
        result.append(seg);
    }

    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <swc_file> [maxres_x maxres_y maxres_z]" << std::endl;
        std::cout << "  Reads an SWC/ESWC file and runs all detection algorithms." << std::endl;
        return 1;
    }

    std::string filepath = argv[1];
    std::cout << "Reading: " << filepath << std::endl;

    V_NeuronSWC_list segList = readSWCFile(filepath);
    std::cout << "Loaded " << segList.seg.size() << " segments, " << segList.nrows() << " total nodes." << std::endl;

    if (segList.seg.size() == 0) {
        std::cerr << "No segments loaded. Check file format." << std::endl;
        return 1;
    }

    // Optional: parse image max resolution
    XYZ maxRes(0, 0, 0);
    const XYZ* pMaxRes = nullptr;
    if (argc >= 5) {
        maxRes.x = std::stof(argv[2]);
        maxRes.y = std::stof(argv[3]);
        maxRes.z = std::stof(argv[4]);
        pMaxRes = &maxRes;
        std::cout << "Image max resolution: " << maxRes.x << " x " << maxRes.y << " x " << maxRes.z << std::endl;
    }

    // Build point-to-segment map for tip detection
    auto allPoint2SegIdMap = swcdetection::getWholeGrid2SegIDMap(segList);

    // Run all detections
    std::cout << "\n--- SpecStructs Detection ---" << std::endl;
    auto specResults = swcdetection::specStructsDetection(segList);
    std::cout << "Found " << specResults.size() << " issues:" << std::endl;
    for (auto& r : specResults) {
        std::cout << "  [" << r.description << "] at (" << r.x << ", " << r.y << ", " << r.z << ") type=" << r.type << std::endl;
    }

    std::cout << "\n--- Loop Detection ---" << std::endl;
    auto loopResults = swcdetection::loopDetection(segList);
    std::cout << "Found " << loopResults.size() << " loops:" << std::endl;
    for (auto& r : loopResults) {
        std::cout << "  [" << r.description << "] at (" << r.x << ", " << r.y << ", " << r.z << ")" << std::endl;
    }

    std::cout << "\n--- Tip Detection ---" << std::endl;
    auto tipResults = swcdetection::tipDetection(segList, allPoint2SegIdMap, 30, nullptr, pMaxRes);
    std::cout << "Found " << tipResults.size() << " tips:" << std::endl;
    for (auto& r : tipResults) {
        std::cout << "  [" << r.description << "] at (" << r.x << ", " << r.y << ", " << r.z << ")" << std::endl;
    }

    std::cout << "\n--- Branching Detection ---" << std::endl;
    auto branchResults = swcdetection::branchingDetection(segList, 12, pMaxRes);
    std::cout << "Found " << branchResults.size() << " branching points:" << std::endl;
    for (auto& r : branchResults) {
        std::cout << "  [" << r.description << "] at (" << r.x << ", " << r.y << ", " << r.z << ")" << std::endl;
    }

    std::cout << "\n--- Crossing Detection ---" << std::endl;
    auto crossResults = swcdetection::crossingDetection(segList, pMaxRes);
    std::cout << "Found " << crossResults.size() << " crossings:" << std::endl;
    for (auto& r : crossResults) {
        std::cout << "  Branch1: (" << r.x1 << ", " << r.y1 << ", " << r.z1 << ")"
                  << " <-> Branch2: (" << r.x2 << ", " << r.y2 << ", " << r.z2 << ")" << std::endl;
    }

    std::cout << "\nDone." << std::endl;
    return 0;
}
