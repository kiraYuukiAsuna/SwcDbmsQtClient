#pragma once

#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <limits>
#include <span>

// V3DLONG: platform-appropriate long type
#if defined(_MSC_VER) && defined(_WIN64)
#define V3DLONG long long
#else
#define V3DLONG long
#endif

// c_array: lightweight C-array wrapper
template<class ELT, int SIZE>
class c_array {
public:
    typedef ELT value_type;
    typedef int size_type;
    typedef int index_type;
    typedef ELT* iterator;
    typedef ELT const* const_iterator;

    operator ELT*() { return data; }
    operator const ELT*() const { return data; }

    static size_type size() { return SIZE; }

    iterator begin() { return &data[0]; }
    const_iterator begin() const { return &data[0]; }
    iterator end() { return &data[SIZE]; }
    const_iterator end() const { return &data[SIZE]; }

private:
    ELT data[SIZE];
};

// RGB8
struct RGB8 {
    union {
        struct { unsigned char r, g, b; };
        c_array<unsigned char, 3> c;
    };
};

// RGBA8
struct RGBA8 {
    union {
        struct { unsigned char r, g, b, a; };
        c_array<unsigned char, 4> c;
        unsigned int i;
    };
};

// XYZ
struct XYZ {
    union {
        struct { float x, y, z; };
        c_array<float, 3> v;
    };

    XYZ(float px, float py, float pz) { x = px; y = py; z = pz; }
    XYZ(float a = 0) { x = a; y = a; z = a; }
};

inline bool operator==(const XYZ& a, const XYZ& b) {
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

inline XYZ operator+(const XYZ& a, const XYZ& b) {
    return XYZ(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline XYZ operator-(const XYZ& a, const XYZ& b) {
    return XYZ(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline float dot(const XYZ& a, const XYZ& b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

inline float norm(const XYZ& a) {
    return std::sqrt(dot(a, a));
}

inline float dist_L2(const XYZ& a, const XYZ& b) {
    XYZ c(a.x - b.x, a.y - b.y, a.z - b.z);
    return std::sqrt(dot(c, c));
}

// V_NeuronSWC_coord
struct V_NeuronSWC_coord {
    union {
        double data[3];
        struct { double x, y, z; };
    };
    bool equal(const V_NeuronSWC_coord& other) const { return x == other.x && y == other.y && z == other.z; }
    void set(double x1, double y1, double z1) { x = x1; y = y1; z = z1; }
};

inline bool operator==(const V_NeuronSWC_coord a, const V_NeuronSWC_coord b) {
    return (a.x == b.x && a.y == b.y && a.z == b.z);
}

inline bool operator<(const V_NeuronSWC_coord a, const V_NeuronSWC_coord b) {
    return ((a.x < b.x) ||
            (a.x == b.x && a.y < b.y) ||
            (a.x == b.x && a.y == b.y && a.z < b.z));
}

// V_NeuronSWC_unit
struct V_NeuronSWC_unit {
    union {
        double data[10];
        struct {
            double n, type, x, y, z, r, parent,
                nchild,
                seg_id, nodeinseg_id,
                level, creatmode, timestamp, tfresindex;
        };
    };
    V_NeuronSWC_unit() {
        for (V3DLONG i = 0; i < (V3DLONG)(sizeof(data) / sizeof(double)); i++)
            data[i] = 0;
        r = 0.5;
    }
    operator V_NeuronSWC_coord() { V_NeuronSWC_coord c; c.x = x; c.y = y; c.z = z; return c; }
    V_NeuronSWC_coord get_coord() { V_NeuronSWC_coord c; c.x = x; c.y = y; c.z = z; return c; }
    void set(double x1, double y1, double z1, double r1, double p1, double t1) { x = x1; y = y1; z = z1; r = r1; parent = p1; type = t1; }
    void set(double x1, double y1, double z1, double r1, double p1) { x = x1; y = y1; z = z1; r = r1; parent = p1; }
    void set(double x1, double y1, double z1, double r1) { x = x1; y = y1; z = z1; r = r1; }
    void set(double x1, double y1, double z1) { x = x1; y = y1; z = z1; }
    void setType(double t) { type = t; }

    std::string uuid;
};

// V_NeuronSWC
struct V_NeuronSWC {
    std::vector<V_NeuronSWC_unit> row;
    bool b_linegraph;
    std::string name;
    std::string comment;
    std::string file;
    unsigned char color_uc[4];
    bool b_jointed;
    bool to_be_deleted;
    bool to_be_broken;
    bool on;

    V_NeuronSWC(std::string new_name = "unset", bool is_linegraph = false) {
        name = new_name; b_linegraph = is_linegraph; *(int*)color_uc = 0; b_jointed = false;
        to_be_deleted = false;
        to_be_broken = false;
        on = true;
    }

    V3DLONG nrows() { return row.size(); }
    void append(V_NeuronSWC_unit& new_node) { row.push_back(new_node); }
    void clear() { row.clear(); }

    bool reverse() {
        V3DLONG N = row.size();
        if (N <= 0) return false;
        // Simple reverse: swap data and fix parent links
        std::vector<V_NeuronSWC_unit> new_row(N);
        for (V3DLONG i = 0; i < N; i++) {
            new_row[i] = row[N - 1 - i];
            new_row[i].n = i + 1;
            new_row[i].parent = (i >= N - 1) ? -1 : (i + 2);
        }
        row = new_row;
        return true;
    }

    void printInfo() {
        // No-op in standalone version
    }
};

// V_NeuronSWC_list
struct V_NeuronSWC_list {
    std::vector<V_NeuronSWC> seg;
    V3DLONG last_seg_num;
    std::string name;
    std::string comment;
    std::string file;
    unsigned char color_uc[4];
    bool b_traced;

    V_NeuronSWC_list() { last_seg_num = -1; *(int*)color_uc = 0; b_traced = true; }

    V3DLONG nsegs() { return seg.size(); }
    V3DLONG nrows() { V3DLONG n = 0; for (V3DLONG i = 0; i < (V3DLONG)seg.size(); i++) n += seg.at(i).nrows(); return n; }

    void append(V_NeuronSWC& new_seg) { seg.push_back(new_seg); last_seg_num = seg.size(); }
    void clear() { last_seg_num = seg.size(); seg.clear(); }
};

// Simplified NeuronSWC for detection output (Qt-free)
struct NeuronSWC {
    V3DLONG n;
    int type;
    float x, y, z;
    float r;
    V3DLONG pn;

    NeuronSWC() : n(0), type(0), x(0), y(0), z(0), r(0), pn(0) {}

    operator XYZ() const { return XYZ(x, y, z); }
};

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
