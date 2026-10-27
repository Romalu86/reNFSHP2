#include "camera_director_runtime.hpp"
namespace nfs::game::world::camera {
using kernel::numeric::Extended;

void pitch_basis(const float *input, float angle, float *output) {
    float copy[9];
    if (input == output) {
        std::memcpy(copy, input, sizeof(copy));
        input = copy;
    }
    float sine, cosine;
    runtime::turn_sine_cosine(&angle, &sine, &cosine);
    const auto format = kernel::numeric::current_format();
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = input[axis];
    for (unsigned axis = 0; axis < 3; ++axis)
        output[3 + axis] = float(Extended(cosine, format) * input[3 + axis] +
                                 Extended(sine, format) * input[6 + axis]);
    for (unsigned axis = 0; axis < 3; ++axis)
        output[6 + axis] = float(Extended(cosine, format) * input[6 + axis] -
                                 Extended(sine, format) * input[3 + axis]);
}

void axis_rotation(const float *axis, float angle, float *output) {
    if (angle == 0 && !std::isnan(angle)) {
        const float identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
        std::memcpy(output, identity, sizeof(identity));
        return;
    }
    float sine, cosine, direction[3];
    runtime::turn_sine_cosine(&angle, &sine, &cosine);
    runtime::normalize_vector(axis, direction);
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    const float remainder = float(X(1.f) - cosine);
    const float x = float(X(direction[0]) * remainder), y = float(X(direction[1]) * remainder),
                z = float(X(direction[2]) * remainder);
    output[0] = float(X(direction[0]) * x + cosine);
    output[1] = float(X(direction[1]) * x + X(direction[2]) * sine);
    output[2] = float(X(direction[2]) * x - X(direction[1]) * sine);
    output[3] = float(X(y) * direction[0] - X(direction[2]) * sine);
    output[4] = float(X(direction[1]) * y + cosine);
    output[5] = float(X(direction[2]) * y + X(direction[0]) * sine);
    output[6] = float(X(z) * direction[0] + X(direction[1]) * sine);
    output[7] = float(X(z) * direction[1] - X(direction[0]) * sine);
    output[8] = float(X(direction[2]) * z + cosine);
}
} // namespace nfs::game::world::camera
