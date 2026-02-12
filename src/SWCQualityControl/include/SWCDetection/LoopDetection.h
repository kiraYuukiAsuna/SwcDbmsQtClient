#pragma once

#include "types.h"
#include <vector>

namespace swcdetection {

// Detect loops in SWC segment list.
// Returns detected loop points (type=0).
std::vector<DetectionResult> loopDetection(
    V_NeuronSWC_list& inputSegList,
    double dist_thresh = 8
);

} // namespace swcdetection
