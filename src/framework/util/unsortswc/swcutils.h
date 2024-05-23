#pragma once

#include "basic_c_fun/basic_surf_objs.h"
#include "neuron_editing/neuron_format_converter.h"
#include "neuron_editing/v_neuronswc.h"

void convertSWC2UnSorted(QString sortedSwcPath, QString unSortedSwcPath){
    auto nt = readSWC_file(sortedSwcPath);
    auto segments = NeuronTree__2__V_NeuronSWC_list(nt);
    writeESWC_file(unSortedSwcPath, V_NeuronSWC_list__2__NeuronTree(segments));
}
