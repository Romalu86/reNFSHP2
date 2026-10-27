#pragma once

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "These formulas require the native x87 intermediate format");

namespace nfs::game::runtime::hardware {
// keep the three products and z + (y + x) in ST precision.
inline long double dot_product(const float *first, const float *second) {
    const auto x = static_cast<long double>(first[0]) * second[0];
    const auto y = static_cast<long double>(first[1]) * second[1];
    const auto z = static_cast<long double>(first[2]) * second[2];
    return z + (y + x);
}

// y, x, z loads, then sqrt((x*x + y*y) + z*z).
inline long double vector_length(const float *value) {
    const auto y = static_cast<long double>(value[1]);
    const auto x = static_cast<long double>(value[0]);
    const auto z = static_cast<long double>(value[2]);
    const auto squared_x = x * x;
    const auto squared_y = y * y;
    const auto squared_xy = squared_x + squared_y;
    const auto squared_z = z * z;
    return __builtin_sqrtl(squared_xy + squared_z);
}

// retain each difference, then add (z*z + y*y) + x*x.
inline long double distance_squared(const float *first, const float *second) {
    const auto x = static_cast<long double>(second[0]) - first[0];
    const auto y = static_cast<long double>(second[1]) - first[1];
    const auto z = static_cast<long double>(second[2]) - first[2];
    const auto zz = z * z;
    const auto yy = y * y;
    const auto squared_zy = zz + yy;
    const auto xx = x * x;
    return squared_zy + xx;
}

// both squared length and length have explicit F32 stores.
inline float normalize_collision(float *value) {
    const auto z = static_cast<long double>(value[2]);
    const auto y = static_cast<long double>(value[1]);
    const auto x = static_cast<long double>(value[0]);
    const float squared = float((x * x + y * y) + z * z);
    if (squared == 0.0f)
        return 0.0f;
    const float length = float(__builtin_sqrtl(static_cast<long double>(squared)));
    const auto inverse = 1.0L / length;
    value[0] = float(inverse * value[0]);
    value[1] = float(inverse * value[1]);
    value[2] = float(inverse * value[2]);
    return length;
}
} // namespace nfs::game::runtime::hardware
