#pragma once

#include "types.h"
#include <vector>

namespace swcdetection {

// Detect bifurcation (type=6) and multifurcation (type=8) points.
// somaCoordinate: if non-null, bifurcation points near soma are filtered out.
std::vector<DetectionResult> specStructsDetection(
    V_NeuronSWC_list& inputSegList,
    double dist_thresh = 0.2,
    const XYZ* somaCoordinate = nullptr
);

} // namespace swcdetection
