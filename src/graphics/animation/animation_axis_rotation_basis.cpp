#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "animation_axis_rotation.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
float rotation_angle_span;
 
void initialize_rotation_angle_span() {
    auto format = kernel::numeric::current_format();
    rotation_angle_span = float(Extended(3.1415927410125732421875f, format) - -3.1415927410125732421875f);
}

void initialize_axis_rotation_constants() {
    initialize_rotation_angle_span();
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_axis_rotation_constants, ".CRT$XCU_NFS_00690624",
                       initialize_axis_rotation_constants);
} // namespace

void __fastcall decode_axis_rotation_descriptor(const std::uint16_t *encoded, void *,
                                                AxisRotationDescriptor *out) {
    auto format = kernel::numeric::current_format();
    auto I = [&](unsigned value) { return Extended::integer(value, format); };
    auto unit = Extended(0.00001525902189314365386962890625f, format) + 0.00001525902189314365386962890625f;
    out->first_offset = float(I(encoded[2]) * unit - 1.0f);
    out->second_offset = float(I(encoded[3]) * unit - 1.0f);
    out->first_scale = float(I(encoded[4]) * unit);
    out->second_scale = float(I(encoded[5]) * unit);
    out->axis = Byte(encoded[6]);
    out->first_angle = float(I(encoded[0]) * rotation_angle_span * 0.00001525902189314365386962890625f + -3.1415927410125732421875f);
    out->second_angle =
        float(I(encoded[1]) * rotation_angle_span * 0.00001525902189314365386962890625f + -3.1415927410125732421875f);
}

void quaternion_from_rotation_angles(const float *angles, float *out) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto trigonometric = [&](unsigned axis, bool sine) {
        auto angle = X(angles[axis]) * .5f;
        if (axis == 2)
            angle = X(float(angle)); // Store the third half-angle before continuing.
        return Extended::from_double(sine ? std::sin(double(angle)) : std::cos(double(angle)),
                                     format);
    };
    float cosine_x = float(trigonometric(0, false)), cosine_y = float(trigonometric(1, false));
    float cosine_z = float(trigonometric(2, false)), sine_x = float(trigonometric(0, true));
    auto sine_y = trigonometric(1, true), sine_z = trigonometric(2, true);
    float cosine_zx = float(X(cosine_z) * cosine_x);
    auto cosine_x_sine_z = X(cosine_x) * sine_z;
    auto sine_x_cosine_z = X(sine_x) * cosine_z;
    float sine_zx = float(sine_z * sine_x);
    out[0] = float(X(cosine_y) * sine_x_cosine_z - cosine_x_sine_z * sine_y);
    out[1] = float(X(sine_zx) * cosine_y + X(cosine_zx) * sine_y);
    out[2] = float(cosine_x_sine_z * cosine_y - sine_x_cosine_z * sine_y);
    out[3] = float(X(cosine_zx) * cosine_y + X(sine_zx) * sine_y);
}

void compose_x_axis_rotation(const float *axis, const float *basis, float *out) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    out[0] = float(X(axis[0]) * basis[3] + X(axis[3]) * basis[0]);
    out[1] = float(X(axis[3]) * basis[1] + X(basis[2]) * axis[0]);
    out[2] = float(X(axis[3]) * basis[2] - X(basis[1]) * axis[0]);
    out[3] = float(X(axis[3]) * basis[3] - X(axis[0]) * basis[0]);
}

void compose_y_axis_rotation(const float *first_basis, const float *axis, const float *second_basis,
                             float *out) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    auto x = X(first_basis[0]) * axis[3], y = X(axis[1]) * first_basis[3];
    float z = float(-(X(axis[1]) * first_basis[0])), w = float(X(first_basis[3]) * axis[3]);
    out[0] = float(x * second_basis[3] - y * second_basis[2]);
    out[1] = float(y * second_basis[3] + x * second_basis[2]);
    out[2] = float(X(w) * second_basis[2] + X(z) * second_basis[3]);
    out[3] = float(X(w) * second_basis[3] - X(z) * second_basis[2]);
}

void compose_z_axis_rotation(const float *basis, const float *axis, float *out) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float v) { return Extended(v, format); };
    out[0] = float(X(basis[0]) * axis[3] - X(axis[2]) * basis[1]);
    out[1] = float(X(basis[1]) * axis[3] + X(axis[2]) * basis[0]);
    out[2] = float(X(axis[2]) * basis[3] + X(basis[2]) * axis[3]);
    out[3] = float(X(basis[3]) * axis[3] - X(axis[2]) * basis[2]);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
