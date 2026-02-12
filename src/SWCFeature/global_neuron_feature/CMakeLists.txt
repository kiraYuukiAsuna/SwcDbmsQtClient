add_executable(global_neuron_feature
    main.cpp
    compute_morph.cpp
)
target_link_libraries(global_neuron_feature PRIVATE swc_common)
