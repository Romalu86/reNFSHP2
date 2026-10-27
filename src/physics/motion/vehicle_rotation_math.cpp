#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include <cmath>
#include <cstring>
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
namespace nfs::game::runtime::physics {

void matrix_to_quaternion(const float *matrix, float *output) {
    auto X = [](float value) { return static_cast<long double>(value); };
    const auto sum = (X(matrix[4]) + matrix[8]) + matrix[0];
    if (X(0) < sum) {
        const float trace = float(__builtin_sqrtl(X(float(sum + 1.0f))));
        output[3] = trace * 0.5f;
        const auto inverse = X(0.5f) / X(trace);
        output[0] = float((X(matrix[5]) - matrix[7]) * inverse);
        output[1] = float((X(matrix[6]) - matrix[2]) * inverse);
        output[2] = float((X(matrix[1]) - matrix[3]) * inverse);
        return;
    }
    constexpr int next[3] = {1, 2, 0};
    unsigned largest = matrix[0] < matrix[4] ? 1 : 0;
    if (matrix[largest * 3 + largest] < matrix[8])
        largest = 2;
    const int second = next[largest], third = next[second];
    float scale = float(__builtin_sqrtl(X(float((X(matrix[largest * 3 + largest]) -
                                   (X(matrix[third * 3 + third]) + matrix[second * 3 + second])) +
                                  1.0f))));
    float result[4];
    result[largest] = scale * 0.5f;
    const auto inverse = scale != 0 ? X(0.5f) / X(scale) : X(scale);
    result[3] = float((X(matrix[second * 3 + third]) - matrix[third * 3 + second]) * inverse);
    result[second] =
        float((X(matrix[second * 3 + largest]) + matrix[largest * 3 + second]) * inverse);
    result[third] = float((X(matrix[largest * 3 + third]) + matrix[third * 3 + largest]) * inverse);
    std::memcpy(output, result, 16);
}

void quaternion_to_matrix(const float *quaternion, float *output) {
    auto X = [](float value) { return static_cast<long double>(value); };
    const auto x = X(quaternion[0]), y = X(quaternion[1]), z = X(quaternion[2]),
               w = X(quaternion[3]);
    const auto twice_x = x + x, twice_y = y + y;
    const float twice_z = float(z + z);
    const float xw = float(twice_x * w), yw = float(twice_y * w), zw = float(X(twice_z) * w);
    const float xx = float(twice_x * x), xy = float(twice_y * x), xz = float(X(twice_z) * x);
    const auto yy = twice_y * y, yz = X(twice_z) * y, zz = X(twice_z) * z;
    output[0] = float((X(1) - yy) - zz);
    output[1] = float(X(xy) + zw);
    output[2] = float(X(xz) - yw);
    output[3] = float(X(xy) - zw);
    const auto diagonal = X(1) - xx;
    const float stored_diagonal = float(diagonal);
    output[4] = float(diagonal - zz);
    output[5] = float(X(xw) + yz);
    output[6] = float(X(xz) + yw);
    output[7] = float(yz - X(xw));
    output[8] = float(X(stored_diagonal) - yy);
}

void interpolate_rotation(const float *source, const float *target, float fraction, float *output) {
    auto X = [](float value) { return static_cast<long double>(value); };
    auto cosine = ((X(target[3]) * source[3] + X(target[2]) * source[2]) +
                   X(source[0]) * target[0]) + X(target[1]) * source[1];
    float reversed[4];
    if (cosine < X(0.f)) {
        cosine = -cosine;
        for (unsigned i = 0; i < 4; ++i)
            reversed[i] = -target[i];
        target = reversed;
    }
    double source_weight;
    auto target_weight = X(fraction);
    if (static_cast<long double>(0.0001) < X(1.f) - cosine) {
        const float stored_cosine = float(cosine);
        const float angle = float(network_rotation_angle_retained(stored_cosine));
        const float sine = float(actor_sine_retained(angle));
        const double inverse_sine = double(X(1.f) / X(sine));
        const float source_angle = float((X(1.f) - X(fraction)) * X(angle));
        source_weight = double(X(float(actor_sine_retained(source_angle))) *
                               static_cast<long double>(inverse_sine));
        const float target_angle = float(X(fraction) * X(angle));
        target_weight = X(float(actor_sine_retained(target_angle))) *
                        static_cast<long double>(inverse_sine);
    } else {
        source_weight = double(X(1.f) - X(fraction));
    }
    for (unsigned i = 0; i < 4; ++i)
        output[i] = float(X(target[i]) * target_weight +
                          X(source[i]) * static_cast<long double>(source_weight));
}
} // namespace nfs::game::runtime::physics
