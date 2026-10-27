#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <initializer_list>
namespace nfs::game::world::dependency {
namespace channels {
float rotation_scratch[4]{}, translation_scratch[3]{};
namespace {
kernel::numeric::Extended euler_quaternion(const float *&cursor, float *output) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    const float degrees[3] = {*cursor++, *cursor++, *cursor++};
    const float pi = 3.1415927410125732421875f, degree_fraction = 0.0055555556900799274444580078125f;
    // Stores precede FCOS/FSIN. Their exact x87 status and transcendental rounding remain open.
     
    float x = float(X(pi) * degree_fraction * degrees[0] * .5f);
    auto y = X(pi) * degree_fraction * degrees[1] * .5f;
    auto z = X(float(X(pi) * degree_fraction * degrees[2])) * .5f;
    float cx = float(std::cos(static_cast<double>(x)));
    float cy = float(std::cos(static_cast<double>(y)));
    float cz = float(std::cos(static_cast<double>(z)));
    float sx = float(std::sin(static_cast<double>(x)));
    float sy = float(std::sin(static_cast<double>(y)));
    const double sine_z = std::sin(static_cast<double>(z));
    auto cosine_product = X(cz) * cx;
    float sine_product = float(Extended::from_double(sine_z, format) * sx);
    auto z_cosine = Extended::from_double(sine_z, format) * cx;
    auto x_cosine = X(sx) * cz;
    output[0] = float(X(cy) * x_cosine - z_cosine * sy);
    output[1] = float(X(sine_product) * cy + cosine_product * sy);
    output[2] = float(z_cosine * cy - x_cosine * sy);
    auto w = cosine_product * cy + X(sine_product) * sy;
    output[3] = float(w);
    return w;
}
void blend_rotation(float fraction, float *output, bool euler,
                    const kernel::numeric::Extended &second_w) {
    const auto format = kernel::numeric::current_format();
    using kernel::numeric::Extended;
    auto X = [&](float value) { return Extended(value, format); };
    auto dot = euler ? ((X(rotation_scratch[3]) * second_w +
                        X(output[2]) * rotation_scratch[2]) +
                       X(output[1]) * rotation_scratch[1]) +
                          X(output[0]) * rotation_scratch[0]
                     : ((X(rotation_scratch[3]) * output[3] +
                         X(rotation_scratch[1]) * output[1]) +
                        X(rotation_scratch[2]) * output[2]) +
                           X(rotation_scratch[0]) * output[0];
    bool opposite = !(X(0) < dot);
    for (unsigned i = 0; i < 3; ++i)
        output[i] =
            opposite
                ? float(X(rotation_scratch[i]) - (X(output[i]) + rotation_scratch[i]) * fraction)
                : float((X(output[i]) - rotation_scratch[i]) * fraction + rotation_scratch[i]);
    output[3] = float(opposite ? X(rotation_scratch[3]) -
                                    (second_w + rotation_scratch[3]) * fraction
                              : (second_w - rotation_scratch[3]) * fraction +
                                    rotation_scratch[3]);
    auto length = ((X(output[0]) * output[0] + X(output[1]) * output[1]) +
                   X(output[2]) * output[2]) +
                  X(output[3]) * output[3];
    auto inverse = X(1) / length.square_root();
    for (unsigned i = 0; i < 4; ++i)
        output[i] = float(inverse * output[i]);
}
} // namespace

void read_euler_rotation(const float **cursor, float *output) {
    euler_quaternion(*cursor, output);
}

void read_quaternion(const float **cursor, float *output) {
    for (unsigned i = 0; i < 4; ++i) {
        output[i] = **cursor;
        ++*cursor;
    }
}

void read_translation(const float **cursor, float *output) {
    for (unsigned i = 4; i < 7; ++i) {
        output[i] = **cursor;
        ++*cursor;
    }
}

void interpolate_euler_rotation(float fraction, const float **first, const float **second,
                                float *output) {
    euler_quaternion(*first, rotation_scratch);
    auto second_w = euler_quaternion(*second, output);
    blend_rotation(fraction, output, true, second_w);
}

void interpolate_quaternion(float fraction, const float **first, const float **second,
                            float *output) {
    read_quaternion(first, rotation_scratch);
    read_quaternion(second, output);
    blend_rotation(fraction, output, false,
                   kernel::numeric::Extended(output[3], kernel::numeric::current_format()));
}

void interpolate_translation(float fraction, const float **first, const float **second,
                             float *output) {
    for (unsigned i = 0; i < 3; ++i) {
        translation_scratch[i] = **first;
        ++*first;
    }
    read_translation(second, output);
    const auto format = kernel::numeric::current_format();
    for (unsigned i = 0; i < 3; ++i)
        output[4 + i] = float(
            (kernel::numeric::Extended(output[4 + i], format) - translation_scratch[i]) * fraction +
            translation_scratch[i]);
}
} // namespace channels
namespace pending {

void construct_uncompressed_animation(void *animation) {
    using namespace channels;
    Dword *signatures = reinterpret_cast<Dword *>(offset(animation, 16));
    const Dword sample[] = {reinterpret_cast<Dword>(read_quaternion),
                           reinterpret_cast<Dword>(read_euler_rotation),
                           reinterpret_cast<Dword>(read_translation)};
    const Dword interpolate[] = {reinterpret_cast<Dword>(interpolate_quaternion),
                                reinterpret_cast<Dword>(interpolate_euler_rotation),
                                reinterpret_cast<Dword>(interpolate_translation)};
    for (const Dword *functions : {sample, interpolate}) {
        int consumed = 0;
        while (consumed < at<int>(animation, 4)) {
            int count = int(*signatures++);
            consumed = int(Dword(consumed) + 1);
            if (count > 0) {
                consumed = int(Dword(consumed) + Dword(count));
                do {
                    Dword kind = *signatures;
                    if (kind < 3)
                        *signatures = functions[kind];
                    else
                        runtime::particles::lifecycle::pending::diagnostic_message(
                            "Bad signature channel type\n");
                    ++signatures;
                } while (--count);
            }
        }
        signatures = reinterpret_cast<Dword *>(offset(animation, 16 + at<Dword>(animation, 4) * 4));
    }
}
} // namespace pending
} // namespace nfs::game::world::dependency
