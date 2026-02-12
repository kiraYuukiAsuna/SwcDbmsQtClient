add_executable(pruning_swc
    main.cpp
    pruning_swc_algo.cpp
)
target_link_libraries(pruning_swc PRIVATE swc_common)
