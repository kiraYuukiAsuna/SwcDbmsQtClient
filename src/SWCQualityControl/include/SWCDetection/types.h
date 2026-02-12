#pragma once

// Use the project's canonical types from unsortswc instead of duplicating them.
#include "basic_c_fun/basic_surf_objs.h"
#include "neuron_editing/v_neuronswc.h"

#include <cmath>
#include <functional>
#include <unordered_map>
#include <unordered_set>

// Detection output types
struct DetectionResult {
    float x, y, z;
    int type;
    std::string description;
};

struct CrossingResult {
    float x1, y1, z1;
    float x2, y2, z2;
};

// Crossing detection types (from detect_crossing/)
namespace util {
    struct Vec3 {
        float x, y, z;
    };

    struct Node {
        int n{1}, parent{-1};
        float x{0.0}, y{0.0}, z{0.0};

        bool operator==(const Node& other) const {
            return n == other.n &&
                   parent == other.parent &&
                   x == other.x &&
                   y == other.y &&
                   z == other.z;
        }
    };

    inline float neuronDistance(const Node& n1, const Node& n2) {
        auto dx = n1.x - n2.x;
        auto dy = n1.y - n2.y;
        auto dz = n1.z - n2.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    inline std::unordered_map<int, std::vector<int>> buildChildMap(const std::vector<Node>& nodes) {
        std::unordered_map<int, std::vector<int>> childMap;
        for (const auto& node : nodes) {
            childMap[node.parent].push_back(node.n);
        }
        return childMap;
    }
}

template<>
struct std::hash<util::Node> {
    size_t operator()(const util::Node& node) const noexcept {
        size_t h1 = std::hash<int>()(node.n);
        size_t h2 = std::hash<int>()(node.parent);
        size_t h3 = std::hash<float>()(node.x);
        size_t h4 = std::hash<float>()(node.y);
        size_t h5 = std::hash<float>()(node.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
    }
};
