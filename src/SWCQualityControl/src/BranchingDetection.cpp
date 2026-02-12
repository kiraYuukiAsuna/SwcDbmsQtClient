#include "SWCDetection/BranchingDetection.h"
#include "SWCDetection/swc_utils.h"
#include <set>
#include <map>
#include <string>
#include <iostream>

namespace swcdetection {

std::vector<DetectionResult> branchingDetection(
    V_NeuronSWC_list inputSegList,
    double dist_thresh,
    const XYZ* imageMaxRes)
{
    std::vector<DetectionResult> results;
    if (inputSegList.seg.size() == 0)
        return results;

    std::map<std::string, bool> isEndPointMap;
    std::map<std::string, std::set<size_t>> wholeGrid2segIDmap;

    for (size_t i = 0; i < inputSegList.seg.size(); ++i) {
        V_NeuronSWC seg = inputSegList.seg[i];
        for (size_t j = 0; j < seg.row.size(); ++j) {
            std::string gridKey = makeGridKey(seg.row[j].x, seg.row[j].y, seg.row[j].z);
            wholeGrid2segIDmap[gridKey].insert(i);
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

    // Detect branching points (exactly 3 links)
    for (size_t i = 0; i < points.size(); ++i) {
        if (linksIndex[i].size() == 3) {
            bool isValid = true;
            auto connectedSegsSet = wholeGrid2segIDmap[points[i]];
            if (connectedSegsSet.size() == 3) {
                for (auto segIt = connectedSegsSet.begin(); segIt != connectedSegsSet.end(); segIt++) {
                    double length = getSegLength(inputSegList.seg[*segIt]);
                    std::string gridKey = makeGridKey(
                        inputSegList.seg[*segIt].row[0].x,
                        inputSegList.seg[*segIt].row[0].y,
                        inputSegList.seg[*segIt].row[0].z);
                    if (gridKey == points[i]) {
                        gridKey = makeGridKey(
                            inputSegList.seg[*segIt].row[inputSegList.seg[*segIt].nrows() - 1].x,
                            inputSegList.seg[*segIt].row[inputSegList.seg[*segIt].nrows() - 1].y,
                            inputSegList.seg[*segIt].row[inputSegList.seg[*segIt].nrows() - 1].z);
                    }
                    if (length < dist_thresh && wholeGrid2segIDmap[gridKey].size() == 1) {
                        isValid = false;
                        break;
                    }
                }
            } else if (connectedSegsSet.size() == 2) {
                int wholeSegIndex = -1;
                int specialSegIndex = -1;
                int middleIndex = -1;
                for (auto segIt = connectedSegsSet.begin(); segIt != connectedSegsSet.end(); segIt++) {
                    int index = getPointInSegIndex(points[i], inputSegList.seg[*segIt]);
                    if (index == -1) {
                        isValid = false;
                        break;
                    } else if (index == 0 || index == (int)inputSegList.seg[*segIt].nrows() - 1) {
                        wholeSegIndex = *segIt;
                    } else if (index > 0 && index < (int)inputSegList.seg[*segIt].nrows() - 1) {
                        specialSegIndex = *segIt;
                        middleIndex = index;
                    }
                }
                if (!isValid || wholeSegIndex == -1 || specialSegIndex == -1)
                    continue;

                double length1 = getSegLength(inputSegList.seg[wholeSegIndex]);
                double length2 = getSegLengthBetweenIndexs(inputSegList.seg[specialSegIndex], 0, middleIndex);
                double length3 = getSegLengthBetweenIndexs(inputSegList.seg[specialSegIndex], middleIndex, inputSegList.seg[specialSegIndex].nrows() - 1);

                std::string gridKey = makeGridKey(
                    inputSegList.seg[wholeSegIndex].row[0].x,
                    inputSegList.seg[wholeSegIndex].row[0].y,
                    inputSegList.seg[wholeSegIndex].row[0].z);
                if (gridKey == points[i]) {
                    gridKey = makeGridKey(
                        inputSegList.seg[wholeSegIndex].row[inputSegList.seg[wholeSegIndex].nrows() - 1].x,
                        inputSegList.seg[wholeSegIndex].row[inputSegList.seg[wholeSegIndex].nrows() - 1].y,
                        inputSegList.seg[wholeSegIndex].row[inputSegList.seg[wholeSegIndex].nrows() - 1].z);
                }
                if (length1 < dist_thresh && wholeGrid2segIDmap[gridKey].size() == 1)
                    continue;

                gridKey = makeGridKey(
                    inputSegList.seg[specialSegIndex].row[0].x,
                    inputSegList.seg[specialSegIndex].row[0].y,
                    inputSegList.seg[specialSegIndex].row[0].z);
                if (length2 < dist_thresh && wholeGrid2segIDmap[gridKey].size() == 1)
                    continue;

                gridKey = makeGridKey(
                    inputSegList.seg[specialSegIndex].row[inputSegList.seg[specialSegIndex].nrows() - 1].x,
                    inputSegList.seg[specialSegIndex].row[inputSegList.seg[specialSegIndex].nrows() - 1].y,
                    inputSegList.seg[specialSegIndex].row[inputSegList.seg[specialSegIndex].nrows() - 1].z);
                if (length3 < dist_thresh && wholeGrid2segIDmap[gridKey].size() == 1)
                    continue;
            }

            if (!isValid)
                continue;

            float sx, sy, sz;
            stringToXYZ(points[i], sx, sy, sz);

            bool inBounds = true;
            if (imageMaxRes) {
                inBounds = (sx > 65 && sx + 65 < imageMaxRes->x &&
                            sy > 65 && sy + 65 < imageMaxRes->y &&
                            sz > 65 && sz + 65 < imageMaxRes->z);
            }

            if (inBounds) {
                results.push_back({sx, sy, sz, 7, "Branching point"});
            }
        }
    }
    return results;
}

} // namespace swcdetection
