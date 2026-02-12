#pragma once

#include "types.h"
#include <vector>
#include <map>
#include <set>
#include <string>

namespace swcdetection {

// Detect missing tip points in SWC segment list.
// allPoint2SegIdMap: mapping from grid key to segment IDs (from getWholeGrid2SegIDMap on the full dataset).
// somaCoordinate: if non-null, tips near soma (within 50 units) are excluded.
// imageMaxRes: if non-null, tips near image boundary (within 33 units) are excluded.
std::vector<DetectionResult> tipDetection(
    V_NeuronSWC_list inputSegList,
    const std::map<std::string, std::set<size_t>>& allPoint2SegIdMap,
    double dist_thresh = 30,
    const XYZ* somaCoordinate = nullptr,
    const XYZ* imageMaxRes = nullptr
);

} // namespace swcdetection
