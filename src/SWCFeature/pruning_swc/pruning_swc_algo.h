#ifndef PRUNING_SWC_ALGO_H
#define PRUNING_SWC_ALGO_H

#include "basic_c_fun/basic_surf_objs.h"

NeuronTree prune_swc_simple(NeuronTree nt, double length, bool& pruned);
NeuronTree prune_swc_iterative(NeuronTree nt, double length, bool& pruned);

#endif // PRUNING_SWC_ALGO_H
