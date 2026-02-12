#pragma once

#include "types.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace swcdetection {

double distance(double x1, double x2, double y1, double y2, double z1, double z2);
double getSegLength(V_NeuronSWC& seg);
double getPartOfSegLength(V_NeuronSWC& seg, int index);
double getPartOfSegLength(V_NeuronSWC& seg, int index1, int index2);
double getSegLengthBetweenIndexs(V_NeuronSWC& seg, int low, int high);

void stringToXYZ(const std::string& xyz, float& x, float& y, float& z);
std::string makeGridKey(float x, float y, float z);

int getPointInSegIndex(const std::string& point, V_NeuronSWC& seg);
std::map<std::string, std::set<size_t>> getWholeGrid2SegIDMap(V_NeuronSWC_list& inputSegments);

int isOverlapOfTwoSegs(V_NeuronSWC& seg1, V_NeuronSWC& seg2);
void reverseSeg(V_NeuronSWC& seg);

std::vector<std::string> stringSplit(const std::string& str, char delim);

} // namespace swcdetection
