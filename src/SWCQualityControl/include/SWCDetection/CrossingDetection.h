#pragma once

#include "types.h"
#include <vector>

namespace swcdetection {

// Detect crossing errors in SWC segment list.
// imageMaxRes: if non-null, crossings near image boundary are excluded.
std::vector<CrossingResult> crossingDetection(
    V_NeuronSWC_list& inputSegList,
    const XYZ* imageMaxRes = nullptr
);

} // namespace swcdetection
