cmake_minimum_required(VERSION 3.16)
project(vaa3d_algorithms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_subdirectory(common)
add_subdirectory(pruning_swc)
add_subdirectory(resample_swc)
add_subdirectory(sort_neuron_swc)
add_subdirectory(global_neuron_feature)
