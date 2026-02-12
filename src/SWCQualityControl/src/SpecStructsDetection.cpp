#include "SWCDetection/SpecStructsDetection.h"
#include "SWCDetection/swc_utils.h"
#include <set>
#include <map>
#include <string>
#include <iostream>

namespace swcdetection {

std::vector<DetectionResult> specStructsDetection(
    V_NeuronSWC_list& inputSegList,
    double dist_thresh,
    const XYZ* somaCoordinate)
{
    std::vector<DetectionResult> results;
    if (inputSegList.seg.size() == 0)
        return results;

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

    // Build graph of endpoints and branching points
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

    // Detect multifurcation (>3 links)
    for (size_t i = 0; i < points.size(); ++i) {
        if (linksIndex[i].size() > 3) {
            float x, y, z;
            stringToXYZ(points[i], x, y, z);
            results.push_back({x, y, z, 8, "Multifurcation"});
        }
    }

    // Detect approaching bifurcation (exactly 3 links, paired close together)
    double soma_radius = 30;
    std::set<size_t> pset;

    size_t pre_tip_id = (size_t)-1;
    size_t cur_tip_id = (size_t)-1;

    for (size_t i = 0; i < points.size(); i++) {
        if (linksIndex[i].size() == 3) {
            pre_tip_id = cur_tip_id;
            cur_tip_id = i;
            if (pre_tip_id != (size_t)-1) {
                float n1x, n1y, n1z;
                stringToXYZ(points[pre_tip_id], n1x, n1y, n1z);
                float n2x, n2y, n2z;
                stringToXYZ(points[cur_tip_id], n2x, n2y, n2z);

                // Check that at least 2 connected segs are long enough
                std::set<size_t> n1Segs = wholeGrid2segIDmap[points[pre_tip_id]];
                std::set<size_t> n2Segs = wholeGrid2segIDmap[points[cur_tip_id]];
                int count1 = 0, count2 = 0;
                for (auto it1 = n1Segs.begin(); it1 != n1Segs.end(); it1++) {
                    if (getSegLength(inputSegList.seg[*it1]) > 40)
                        count1++;
                }
                for (auto it2 = n2Segs.begin(); it2 != n2Segs.end(); it2++) {
                    if (getSegLength(inputSegList.seg[*it2]) > 40)
                        count2++;
                }
                if (!(count1 >= 2 && count2 >= 2))
                    continue;

                if (somaCoordinate) {
                    if (distance(n1x, somaCoordinate->x, n1y, somaCoordinate->y, n1z, somaCoordinate->z) > soma_radius
                        && distance(n2x, somaCoordinate->x, n2y, somaCoordinate->y, n2z, somaCoordinate->z) > soma_radius) {
                        double dist = distance(n1x, n2x, n1y, n2y, n1z, n2z);
                        if (distance((n1x + n2x) / 2, somaCoordinate->x, (n1y + n2y) / 2, somaCoordinate->y, (n1z + n2z) / 2, somaCoordinate->z) > 1e-7 && dist < dist_thresh) {
                            pset.insert(pre_tip_id);
                            pset.insert(cur_tip_id);
                        }
                    }
                } else {
                    double dist = distance(n1x, n2x, n1y, n2y, n1z, n2z);
                    if (dist < dist_thresh) {
                        pset.insert(pre_tip_id);
                        pset.insert(cur_tip_id);
                    }
                }
            }
        }
    }

    for (auto it = pset.begin(); it != pset.end(); it++) {
        float x, y, z;
        stringToXYZ(points[*it], x, y, z);
        results.push_back({x, y, z, 6, "Approaching bifurcation"});
    }

    return results;
}

} // namespace swcdetection
