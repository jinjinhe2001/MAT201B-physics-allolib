#pragma once
#include "al/math/al_Mat.hpp"
#include "al/math/al_Vec.hpp"

using namespace al;

inline float max(float a, float b) {
    return a > b ? a : b;
}

Mat4f CrossMatrix(Vec3f v)
{
    Mat4f mat(0, -v[2], v[1], 0,
              v[2], 0, -v[0], 0,
              -v[1], v[0], 0, 0,
              0, 0, 0, 1);
    return mat;
}

Mat4f ScaleMatrix(Vec3f s) {
    Mat4f mat(s[0], 0, 0, 0,
              0, s[1], 0, 0,
              0, 0, s[2], 0,
              0, 0, 0, 1);
    return mat;
}

