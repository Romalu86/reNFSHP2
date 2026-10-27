#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;

float *__stdcall multiply_quaternion_scalar(float *out, const float *a, const float *b) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto x = X(b[3]) * a[0] + X(b[0]) * a[3] + X(b[1]) * a[2] - X(b[2]) * a[1];
    auto y = X(b[3]) * a[1] - X(b[0]) * a[2] + X(b[1]) * a[3] + X(b[2]) * a[0];
    auto z = X(b[3]) * a[2] + X(b[0]) * a[1] - X(b[1]) * a[0] + X(b[2]) * a[3];
    auto w = X(b[3]) * a[3] - X(b[0]) * a[0] - X(b[1]) * a[1] - X(b[2]) * a[2];
    out[0] = float(x);
    out[1] = float(y);
    out[2] = float(z);
    out[3] = float(w);
    return out;
}

float *__stdcall inverse_quaternion_scalar(float *out, const float *input) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float square = float(X(input[0]) * input[0] + X(input[1]) * input[1] + X(input[2]) * input[2] +
                         X(input[3]) * input[3]);
    if (!(0.000000000000000000108420217248550443400745280086994171142578125f < square)) {
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        out[3] = 0;
        return out;
    }
    for (unsigned i = 0; i < 3; ++i) {
        Dword bits;
        std::memcpy(&bits, input + i, 4);
        bits ^= 2147483648u;
        std::memcpy(out + i, &bits, 4);
    }
    out[3] = input[3];
    if (!approximately_equal(square, 1)) {
        auto reciprocal = X(1) / X(square);
        for (unsigned i = 0; i < 4; ++i)
            out[i] = float(reciprocal * out[i]);
    }
    return out;
}

void __stdcall logarithm_quaternion_scalar(float *out, const float *input) {
    auto format = kernel::numeric::current_format();
    if (input[3] < 1) {
        double retained_angle = std::acos(double(input[3]));
        float angle = float(retained_angle), sine = float(std::sin(retained_angle));
        if (!approximately_equal(sine, 0)) {
            auto factor = Extended(angle, format) / Extended(sine, format);
            for (unsigned i = 0; i < 3; ++i)
                out[i] = float(factor * input[i]);
            out[3] = 0;
            return;
        }
    }
    for (unsigned i = 0; i < 3; ++i)
        out[i] = input[i];
    out[3] = 0;
}

float *__stdcall exponential_quaternion_scalar(float *out, const float *input) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    auto square = X(input[0]) * input[0] + X(input[1]) * input[1] + X(input[2]) * input[2];
    float angle = float(square.square_root());
    out[3] = float(std::cos(double(angle)));
    float sine = float(std::sin(double(angle)));
    if (!approximately_equal(sine, 0)) {
        auto factor = X(sine) / X(angle);
        for (unsigned i = 0; i < 3; ++i)
            out[i] = float(factor * input[i]);
    } else if (out != input)
        for (unsigned i = 0; i < 3; ++i)
            out[i] = input[i];
    return out;
}

float *__stdcall quaternion_from_axis_scalar(float *out, const float *axis, float angle) {
    float normalized[3];
    normalize_vector3_bootstrap(normalized, axis);
    auto format = kernel::numeric::current_format();
    float half = float(Extended(angle, format) * .5f);
    out[3] = float(std::cos(double(half)));
    float sine = float(std::sin(double(half)));
    for (unsigned i = 0; i < 3; ++i)
        out[i] = float(Extended(normalized[i], format) * sine);
    return out;
}

void __stdcall quaternion_yaw_pitch_roll_scalar(float *out, float yaw, float pitch, float roll) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float half_roll = float(X(roll) * .5f), half_pitch = float(X(pitch) * .5f),
          half_yaw = float(X(yaw) * .5f);
    float cr = float(std::cos(double(half_roll))), sr = float(std::sin(double(half_roll)));
    float cp = float(std::cos(double(half_pitch))), sp = float(std::sin(double(half_pitch)));
    float cy = float(std::cos(double(half_yaw))), sy = float(std::sin(double(half_yaw)));
    auto sycp = X(sy) * cp, cysp = X(cy) * sp;
    out[0] = float(sycp * sr + X(cr) * cysp);
    out[1] = float(sycp * cr - cysp * sr);
    auto cycp = X(cy) * cp, sysp = X(sy) * sp;
    out[2] = float(cycp * sr - sysp * cr);
    out[3] = float(cycp * cr + sysp * sr);
}

float *__stdcall yaw_pitch_roll_matrix_scalar(float *out, float yaw, float pitch, float roll) {
    float rotation[4];
    quaternion_yaw_pitch_roll_bootstrap(rotation, yaw, pitch, roll);
    quaternion_matrix_bootstrap(out, rotation);
    return out;
}

void __stdcall quaternion_matrix_scalar(float *out, const float *rotation) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float f) { return Extended(f, format); };
    float x = rotation[0], y = rotation[1], z = rotation[2], w = rotation[3];
    float twice_x = float(X(x) * 2.0f), twice_y = float(X(y) * 2.0f);
    auto twice_z = X(z) * 2.0f;
    auto xw = X(twice_x) * w, yw = X(twice_y) * w, zw = twice_z * w;
    float xx = float(X(twice_x) * x), yx = float(X(twice_y) * x), zx = float(twice_z * x);
    float yy = float(X(twice_y) * y), zy = float(twice_z * y);
    auto zz = twice_z * z;
    out[0] = float(X(1) - yy - zz);
    out[1] = float(X(yx) + zw);
    out[2] = float(X(zx) - yw);
    out[3] = 0;
    out[4] = float(X(yx) - zw);
    auto diagonal = X(1) - xx;
    float stored_diagonal = float(diagonal);
    out[5] = float(diagonal - zz);
    out[6] = float(X(zy) + xw);
    out[7] = 0;
    out[8] = float(X(zx) + yw);
    out[9] = float(X(zy) - xw);
    out[10] = float(X(stored_diagonal) - yy);
    out[11] = 0;
    out[12] = 0;
    out[13] = 0;
    out[14] = 0;
    out[15] = 1;
}
} // namespace nfs::graphics::d3dx
