add_library(swc_common STATIC
    basic_surf_objs.cpp
    swc_io.cpp
)
target_include_directories(swc_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(swc_common PUBLIC Qt6::Core)
