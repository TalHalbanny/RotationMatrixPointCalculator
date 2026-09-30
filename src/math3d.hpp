#pragma once

//math functions import

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

struct Vec3 { // 3D vector {x,y,z}
    float x = 0; //x val
    float y = 0; //y val
    float z = 0; //z val

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; } //operator overloading for add
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; } //operator overloading for sub
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; } //operator overloading for mul
    Vec3 operator-() const { return {-x, -y, -z}; } //operator overloading for neg (opposite of current, negative to positive)
};

inline Vec3 operator*(float s, const Vec3& v) { return v * s; }  //scalar mul operator
inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; } //mul of two vectors
inline Vec3 cross(const Vec3& a, const Vec3& b) { //cross mul of two vectors
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline float length(const Vec3& v) { return std::sqrt(dot(v, v)); } //cal length of vector
inline Vec3 normalize(const Vec3& v) { //normalize vector length to 1 or 0 
    const float len = length(v);
    return len < 1e-8f ? Vec3{0, 0, 0} : v * (1.0f / len);
}
inline float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }  //clamp val between lo and hi to get range

enum class Axis { None, X, Y, Z }; //create enum for axes

inline const char* axisName(Axis axis) { //return pointer to string of axis name x y or z
    switch (axis) {
        case Axis::X: return "X";
        case Axis::Y: return "Y";
        case Axis::Z: return "Z";
        default: return "(click an axis)";
    }
}

inline Vec3 axisVector(Axis axis) { //return vector of axis depends on axis value or {0,0,0}
    switch (axis) {
        case Axis::X: return {1, 0, 0};
        case Axis::Y: return {0, 1, 0};
        case Axis::Z: return {0, 0, 1};
        default: return {0, 0, 0};
    }
}

inline float degToRad(float deg) { return deg * 0.017453292519943295f; } //convert degress to radiants

inline Vec3 rotateAroundAxis(const Vec3& point, Axis axis, float degrees) { //rotate point around axis by drgrees cos sin formula by axis val
    const float r = degToRad(degrees);
    const float c = std::cos(r);
    const float s = std::sin(r);
    switch (axis) {
        case Axis::X: return {point.x, point.y * c - point.z * s, point.y * s + point.z * c}; 
        case Axis::Y: return {point.x * c + point.z * s, point.y, -point.x * s + point.z * c};
        case Axis::Z: return {point.x * c - point.y * s, point.x * s + point.y * c, point.z};
        default: return point;
    }
}

inline std::string formatVec(const Vec3& v) { //format vector to string for review of new point
    char buf[96];
    std::snprintf(buf, sizeof(buf), "(%.3f, %.3f, %.3f)", v.x, v.y, v.z);
    return buf;
}
