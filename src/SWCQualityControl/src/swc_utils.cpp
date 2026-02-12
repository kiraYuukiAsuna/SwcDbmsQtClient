#include "SWCDetection/swc_utils.h"
#include <cmath>
#include <sstream>
#include <algorithm>

namespace swcdetection {

double distance(double x1, double x2, double y1, double y2, double z1, double z2) {
    return std::sqrt(
        (x1 - x2) * (x1 - x2) +
        (y1 - y2) * (y1 - y2) +
        (z1 - z2) * (z1 - z2)
    );
}

double getSegLength(V_NeuronSWC& seg) {
    int size = seg.row.size();
    double sum = 0;
    for (int i = 0; i < size - 1; i++) {
        sum += distance(seg.row[i].x, seg.row[i + 1].x, seg.row[i].y, seg.row[i + 1].y, seg.row[i].z, seg.row[i + 1].z);
    }
    return sum;
}

double getPartOfSegLength(V_NeuronSWC& seg, int index) {
    double sum = 0;
    for (int i = 0; i < index; i++) {
        sum += distance(seg.row[i].x, seg.row[i + 1].x, seg.row[i].y, seg.row[i + 1].y, seg.row[i].z, seg.row[i + 1].z);
    }
    return sum;
}

double getPartOfSegLength(V_NeuronSWC& seg, int index1, int index2) {
    double sum = 0;
    for (int i = index1; i < index2; i++) {
        sum += distance(seg.row[i].x, seg.row[i + 1].x, seg.row[i].y, seg.row[i + 1].y, seg.row[i].z, seg.row[i + 1].z);
    }
    return sum;
}

double getSegLengthBetweenIndexs(V_NeuronSWC& seg, int low, int high) {
    double sum = 0;
    for (int i = low; i < high; i++) {
        sum += distance(seg.row[i].x, seg.row[i + 1].x, seg.row[i].y, seg.row[i + 1].y, seg.row[i].z, seg.row[i + 1].z);
    }
    return sum;
}

void stringToXYZ(const std::string& xyz, float& x, float& y, float& z) {
    auto parts = stringSplit(xyz, '_');
    if (parts.size() >= 3) {
        x = std::stof(parts[0]);
        y = std::stof(parts[1]);
        z = std::stof(parts[2]);
    }
}

std::string makeGridKey(float x, float y, float z) {
    return std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
}

int getPointInSegIndex(const std::string& point, V_NeuronSWC& seg) {
    int index = -1;
    for (int i = 0; i < (int)seg.row.size(); i++) {
        std::string gridKey = makeGridKey(seg.row[i].x, seg.row[i].y, seg.row[i].z);
        if (point == gridKey) {
            index = i;
            break;
        }
    }
    return index;
}

std::map<std::string, std::set<size_t>> getWholeGrid2SegIDMap(V_NeuronSWC_list& inputSegments) {
    std::map<std::string, std::set<size_t>> wholeGrid2SegIDMap;

    for (size_t i = 0; i < inputSegments.seg.size(); ++i) {
        V_NeuronSWC& seg = inputSegments.seg[i];

        for (size_t j = 0; j < seg.row.size(); ++j) {
            std::string gridKey = makeGridKey(seg.row[j].x, seg.row[j].y, seg.row[j].z);
            wholeGrid2SegIDMap[gridKey].insert(i);
        }
    }

    return wholeGrid2SegIDMap;
}

int isOverlapOfTwoSegs(V_NeuronSWC& seg1, V_NeuronSWC& seg2) {
    if (seg1.row.size() == 1 || seg2.row.size() == 1) {
        return 0;
    }

    double length1 = getSegLength(seg1);
    double length2 = getSegLength(seg2);
    double minDensity = std::min(length1 / seg1.row.size(), length2 / seg2.row.size());
    double minLength = std::min(length1, length2);
    double mindist = 0.1 + 5 * std::sqrt(minLength) / (std::sqrt(minLength) + 11);
    double mindist_thres = 0.1 + 5 * std::sqrt(minLength) / (std::sqrt(minLength) + 11);

    if (minDensity < 5) {
        mindist = 0.03 + 1.8 * std::sqrt(minLength) / (std::sqrt(minLength) + 11);
        mindist_thres = 0.03 + 1.8 * std::sqrt(minLength) / (std::sqrt(minLength) + 11);
    }

    if (seg1.row.size() == seg2.row.size()) {
        double dist = 0;
        const size_t cnt = seg1.row.size();
        for (size_t i = 0; i < cnt; i++) {
            auto node = seg1.row.at(i);
            dist += std::sqrt(
                std::pow(node.x - seg2.row[i].x, 2)
                + std::pow(node.y - seg2.row[i].y, 2)
                + std::pow(node.z - seg2.row[i].z, 2)
            );
        }
        if (dist / cnt < mindist)
            return 1;

        dist = 0;
        for (size_t i = 0; i < cnt; i++) {
            auto node = seg1.row.at(i);
            dist += std::sqrt(
                std::pow(node.x - seg2.row[cnt - i - 1].x, 2)
                + std::pow(node.y - seg2.row[cnt - i - 1].y, 2)
                + std::pow(node.z - seg2.row[cnt - i - 1].z, 2)
            );
        }
        if (dist / cnt < mindist)
            return 1;

        return 0;
    }

    bool isReverse = false;
    V_NeuronSWC seg_short = seg1;
    V_NeuronSWC seg_long = seg2;
    double length_short = length1;
    double length_long = length2;

    if (seg1.row.size() > seg2.row.size()) {
        seg_short = seg2;
        seg_long = seg1;
        length_short = length2;
        length_long = length1;
        isReverse = true;
    }

    int long_index1 = -1;
    int long_index2 = -1;
    int start_index = -1;
    double mindist1 = 100;
    double mindist2 = 100;
    double mindist_final = 100;
    int seg_short_index = 0;

    for (auto it = seg_long.row.begin(); it != seg_long.row.end(); it++) {
        double dist = distance(it->x, seg_short.row[seg_short.row.size() - 1].x, it->y, seg_short.row[seg_short.row.size() - 1].y, it->z, seg_short.row[seg_short.row.size() - 1].z);
        if (dist < mindist1) {
            mindist1 = dist;
            long_index1 = it - seg_long.row.begin();
        }
    }

    for (auto it = seg_long.row.begin(); it != seg_long.row.end(); it++) {
        double dist = distance(it->x, seg_short.row[0].x, it->y, seg_short.row[0].y, it->z, seg_short.row[0].z);
        if (dist < mindist2) {
            mindist2 = dist;
            long_index2 = it - seg_long.row.begin();
        }
    }

    if (long_index1 == -1 && long_index2 == -1)
        return 0;

    if (mindist1 < mindist2) {
        start_index = long_index1;
        mindist_final = mindist1;
        seg_short_index = seg_short.row.size() - 1;
    } else {
        start_index = long_index2;
        mindist_final = mindist2;
        seg_short_index = 0;
    }

    if (start_index == -1 || mindist_final >= mindist_thres) {
        return 0;
    } else if ((int)seg_long.row.size() - start_index >= (int)seg_short.row.size()) {
        double dist = 0;
        const size_t cnt = seg_short.row.size();
        for (size_t i = 0; i < cnt; i++) {
            V_NeuronSWC_unit node;
            if (seg_short_index == (int)seg_short.row.size() - 1)
                node = seg_short.row.at(cnt - i - 1);
            if (seg_short_index == 0)
                node = seg_short.row.at(i);

            dist += std::sqrt(
                std::pow(node.x - seg_long.row[start_index + i].x, 2)
                + std::pow(node.y - seg_long.row[start_index + i].y, 2)
                + std::pow(node.z - seg_long.row[start_index + i].z, 2)
            );
        }
        if (dist / cnt < mindist_thres) {
            if (!isReverse)
                return (length_short <= length_long) ? 1 : 2;
            else
                return (length_short <= length_long) ? 2 : 1;
        }
    } else if (start_index + 1 >= (int)seg_short.row.size()) {
        double dist = 0;
        const size_t cnt = seg_short.row.size();
        for (size_t i = 0; i < cnt; i++) {
            V_NeuronSWC_unit node;
            if (seg_short_index == (int)seg_short.row.size() - 1)
                node = seg_short.row.at(cnt - i - 1);
            if (seg_short_index == 0)
                node = seg_short.row.at(i);

            dist += std::sqrt(
                std::pow(node.x - seg_long.row[start_index - i].x, 2)
                + std::pow(node.y - seg_long.row[start_index - i].y, 2)
                + std::pow(node.z - seg_long.row[start_index - i].z, 2)
            );
        }
        if (dist / cnt < mindist_thres) {
            if (!isReverse)
                return (length_short <= length_long) ? 1 : 2;
            else
                return (length_short <= length_long) ? 2 : 1;
        }
    }

    return 0;
}

void reverseSeg(V_NeuronSWC& seg) {
    std::reverse(seg.row.begin(), seg.row.end());
    for (int i = 0; i < (int)seg.row.size(); i++) {
        seg.row[i].n = i + 1;
        seg.row[i].parent = i + 2;
    }
    seg.row[seg.row.size() - 1].parent = -1;
}

std::vector<std::string> stringSplit(const std::string& str, char delim) {
    std::stringstream ss(str);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        if (!item.empty())
            elems.push_back(item);
    }
    return elems;
}

} // namespace swcdetection
