#include "SWCDetection/TipDetection.h"
#include "SWCDetection/swc_utils.h"
#include <set>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

namespace swcdetection {

std::vector<DetectionResult> tipDetection(
    V_NeuronSWC_list inputSegList,
    const std::map<std::string, std::set<size_t>>& allPoint2SegIdMap,
    double dist_thresh,
    const XYZ* somaCoordinate,
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

    // Detect tips
    std::set<std::string> tips;

    for (size_t i = 0; i < inputSegList.seg.size(); i++) {
        V_NeuronSWC seg = inputSegList.seg[i];
        float xLabel1 = seg.row[0].x;
        float yLabel1 = seg.row[0].y;
        float zLabel1 = seg.row[0].z;
        std::string gridKey1 = makeGridKey(xLabel1, yLabel1, zLabel1);
        float xLabel2 = seg.row[seg.row.size() - 1].x;
        float yLabel2 = seg.row[seg.row.size() - 1].y;
        float zLabel2 = seg.row[seg.row.size() - 1].z;
        std::string gridKey2 = makeGridKey(xLabel2, yLabel2, zLabel2);

        auto allIt1 = allPoint2SegIdMap.find(gridKey1);
        auto allIt2 = allPoint2SegIdMap.find(gridKey2);
        size_t allCount1 = (allIt1 != allPoint2SegIdMap.end()) ? allIt1->second.size() : 0;
        size_t allCount2 = (allIt2 != allPoint2SegIdMap.end()) ? allIt2->second.size() : 0;

        if (wholeGrid2segIDmap[gridKey1].size() == 1 && allCount1 == 1 && wholeGrid2segIDmap[gridKey2].size() > 1) {
            if (somaCoordinate) {
                double somaDist = std::sqrt(
                    (xLabel1 - somaCoordinate->x) * (xLabel1 - somaCoordinate->x) +
                    (yLabel1 - somaCoordinate->y) * (yLabel1 - somaCoordinate->y) +
                    (zLabel1 - somaCoordinate->z) * (zLabel1 - somaCoordinate->z));
                if (somaDist > 50)
                    tips.insert(gridKey1);
            } else {
                tips.insert(gridKey1);
            }
        }
        if (wholeGrid2segIDmap[gridKey2].size() == 1 && allCount2 == 1 && wholeGrid2segIDmap[gridKey1].size() > 1) {
            if (somaCoordinate) {
                double somaDist = std::sqrt(
                    (xLabel2 - somaCoordinate->x) * (xLabel2 - somaCoordinate->x) +
                    (yLabel2 - somaCoordinate->y) * (yLabel2 - somaCoordinate->y) +
                    (zLabel2 - somaCoordinate->z) * (zLabel2 - somaCoordinate->z));
                if (somaDist > 50)
                    tips.insert(gridKey2);
            } else {
                tips.insert(gridKey2);
            }
        }
    }

    // Check branch length for each tip
    for (auto it = tips.begin(); it != tips.end(); it++) {
        std::vector<size_t> visitedSegIds;
        size_t segId = *wholeGrid2segIDmap[*it].begin();
        visitedSegIds.push_back(segId);
        V_NeuronSWC seg = inputSegList.seg[segId];
        std::string gridKey0 = makeGridKey(seg.row[0].x, seg.row[0].y, seg.row[0].z);
        double tipBranchLength = 0;
        bool isReverse = false;
        if (wholeGrid2segIDmap[gridKey0].size() != 1) {
            isReverse = true;
        }
        bool flag = true;
        while (true) {
            int size = seg.row.size();
            std::vector<int> indexs(size);
            for (int m = 0; m < size; m++)
                indexs[m] = m;
            if (isReverse)
                std::reverse(indexs.begin(), indexs.end());
            for (int i = 0; i < size; i++) {
                int index = indexs[i];
                std::string gridKey = makeGridKey(seg.row[index].x, seg.row[index].y, seg.row[index].z);
                auto it2 = std::find(points.begin(), points.end(), gridKey);
                if (it2 != points.end()) {
                    int index2 = it2 - points.begin();
                    if (linksIndex[index2].size() >= 3) {
                        flag = false;
                        break;
                    } else {
                        if (index == (int)seg.row.size() - 1)
                            break;
                        tipBranchLength += distance(seg.row[index].x, seg.row[index + 1].x,
                                                    seg.row[index].y, seg.row[index + 1].y,
                                                    seg.row[index].z, seg.row[index + 1].z);
                        if (tipBranchLength >= dist_thresh)
                            break;
                        continue;
                    }
                }
                if (index < size - 1) {
                    tipBranchLength += distance(seg.row[index].x, seg.row[index + 1].x,
                                                seg.row[index].y, seg.row[index + 1].y,
                                                seg.row[index].z, seg.row[index + 1].z);
                }
                if (tipBranchLength >= dist_thresh)
                    break;
            }

            if (tipBranchLength >= dist_thresh || !flag)
                break;

            std::string endGridKey = makeGridKey(seg.row[indexs[size - 1]].x, seg.row[indexs[size - 1]].y, seg.row[indexs[size - 1]].z);
            if (wholeGrid2segIDmap[endGridKey].size() != 2) {
                tipBranchLength = 0;
                break;
            }
            for (auto segIt = wholeGrid2segIDmap[endGridKey].begin(); segIt != wholeGrid2segIDmap[endGridKey].end(); segIt++) {
                if (segId != *segIt) {
                    segId = *segIt;
                    break;
                }
            }

            if (std::find(visitedSegIds.begin(), visitedSegIds.end(), segId) == visitedSegIds.end())
                visitedSegIds.push_back(segId);
            else {
                tipBranchLength = 0;
                break;
            }
            seg = inputSegList.seg[segId];
            std::string gridKey2 = makeGridKey(seg.row[0].x, seg.row[0].y, seg.row[0].z);
            if (gridKey2 != endGridKey)
                isReverse = true;
            else
                isReverse = false;
        }

        if (tipBranchLength >= dist_thresh) {
            float sx, sy, sz;
            stringToXYZ(*it, sx, sy, sz);

            bool inBounds = true;
            if (imageMaxRes) {
                inBounds = (sx > 33 && sx + 33 < imageMaxRes->x &&
                            sy > 33 && sy + 33 < imageMaxRes->y &&
                            sz > 33 && sz + 33 < imageMaxRes->z);
            }

            if (inBounds) {
                results.push_back({sx, sy, sz, 10, "Missing tip"});
            }
        }
    }

    return results;
}

} // namespace swcdetection
