cmake_minimum_required(VERSION 3.20)
project(SWCDetection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Library sources
set(LIB_SOURCES
    src/swc_utils.cpp
    src/LoopDetection.cpp
    src/TipDetection.cpp
    src/BranchingDetection.cpp
    src/SpecStructsDetection.cpp
    src/CrossingDetection.cpp
)

add_library(swcdetection STATIC ${LIB_SOURCES})

target_include_directories(swcdetection PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(swcdetection PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/octree
)

# Example executable
add_executable(swcdetection_example main.cpp)
target_link_libraries(swcdetection_example PRIVATE swcdetection)
target_include_directories(swcdetection_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
