#pragma once

#include "types.h"
#include <vector>

namespace swcdetection {

// Detect branching points in SWC segment list.
// imageMaxRes: if non-null, branching points near image boundary (within 65 units) are excluded.
std::vector<DetectionResult> branchingDetection(
    V_NeuronSWC_list inputSegList,
    double dist_thresh = 12,
    const XYZ* imageMaxRes = nullptr
);

} // namespace swcdetection
