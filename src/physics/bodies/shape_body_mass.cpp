#include "shape_body_runtime.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "../motion/world_motion_x87.hpp"
#include <cstring>
namespace nfs::game::runtime::physics {
void report_body_shape_error(Dword line) {
    pending::error_source =
        "C:\\HP2\\Net\\Patch1\\Code\\Source\\Game_Objects\\Common\\NEWTON.c";
    pending::physics_assertion_line = line;
    pending::prepare_error();
    pending::report_runtime_error(pending::shape_error_message);
}

void set_body_material(void *body, Dword material) {
    switch (material) {
    case 0:
        at<float>(body, 56) = 0.1f;
        at<float>(body, 480) = 0.35f;
        break;
    case 1:
        at<float>(body, 56) = 0.05f;
        at<float>(body, 480) = 0.1f;
        break;
    case 2:
        at<float>(body, 56) = 0.15f;
        at<float>(body, 480) = 0.35f;
        break;
    case 3:
        at<float>(body, 56) = 0.35f;
        at<float>(body, 480) = 1.f;
        break;
    case 4:
        at<float>(body, 56) = 0.2f;
        at<float>(body, 480) = 0.5f;
        break;
    case 5:
        at<float>(body, 56) = 0.05f;
        at<float>(body, 480) = 0.7f;
        break;
    case 6:
        at<float>(body, 56) = 0.25f;
        at<Dword>(body, 464) = 1052602532;
        at<float>(body, 480) = 0.5f;
        break;
    default:
        at<Dword>(body, 56) = 0;
        at<Dword>(body, 480) = 1057803469;
        break;
    }
}

void initialize_body_mass(void *body, float mass, float x, float y, float z, int shape,
                          Dword material) {
    const Byte slot = at<Byte>(body, 4);
    pending::reset_vehicle_entry(body);
    at<Byte>(body, 4) = slot;
    if (x < 0.25f)
        x = 0.25f;
    if (y < 0.25f)
        y = 0.25f;
    if (z < 0.25f)
        z = 0.25f;
    if (shape == 4) {
        float maximum = x > y ? x : y;
        if (!(maximum > z))
            maximum = z;
        x = y = z = maximum;
    }
    at<Dword>(body, 40) = material;
    at<int>(body, 136) = shape;
    at<float>(body, 120) = x;
    at<float>(body, 124) = y;
    at<float>(body, 128) = z;
    at<float>(body, 132) =
        float(hardware::vector_length(reinterpret_cast<float *>(bytes(body) + 120)));
    at<float>(body, 32) = mass;
    if (mass == 0.f)
        at<float>(body, 32) = 1000.f;
    auto X = [](float value) { return static_cast<long double>(value); };
    at<float>(body, 36) = float(X(1.f) / X(at<float>(body, 32)));
    const auto retained_xx = X(x) * x, retained_yy = X(y) * y, retained_zz = X(z) * z;
    const float square_x = float(retained_xx), square_y = float(retained_yy),
                square_z = float(retained_zz);
    const float four_x = float(retained_xx * 4.f);
    const float four_y = float(retained_yy * 4.f), four_z = float(retained_zz * 4.f);

    const float side_squared = float(X(square_z) + square_y);
    volatile float side_radius = float(__builtin_sqrtl(X(side_squared)));
    const float radial_squared = float(X(square_z) + square_x);
    const float radius = float(__builtin_sqrtl(X(radial_squared)));
    const float face_squared = float(X(square_y) + square_x);
    volatile float face_radius = float(__builtin_sqrtl(X(face_squared)));
    const float diameter_x = float(X(x) + x);
    const float diameter_y = float(X(y) + y), diameter_z = float(X(z) + z);
    const auto body_mass = X(at<float>(body, 32));
    auto inertia = [&](unsigned offset, long double value) {
        at<float>(body, offset) = float(value);
    };
    if (shape >= 0 && shape <= 5)
        at<Dword>(body, 464) = 1050589266;
    switch (shape) {
    case 0:
        at<float>(body, 468) = at<float>(body, 472) = at<float>(body, 476) = 1.f;
        inertia(452, (X(four_z) + four_y) * body_mass * 0.083333335816860198974609375f);
        inertia(456, (X(four_z) + four_x) * body_mass * 0.083333335816860198974609375f);
        inertia(460, (X(four_y) + four_x) * body_mass * 0.083333335816860198974609375f);
        inertia(68, X(diameter_y) * diameter_x * diameter_z);
        break;
    case 1:
    case 5:
        at<float>(body, 468) = at<float>(body, 476) = 0.5f;
        at<float>(body, 472) = 1.f;
        inertia(452, (X(four_z) * 4.f + X(four_y) * 8.f) * body_mass * 0.0125f);
        inertia(456, (X(four_z) + four_x) * body_mass * 0.05f);
        inertia(460, (X(four_x) * 4.f + X(four_y) * 8.f) * body_mass * 0.0125f);
        inertia(68, X(diameter_x) * diameter_z * diameter_y * 0.25f);
        break;
    case 2:
        at<float>(body, 472) = 1.f;
        if (!(x > z)) {
            at<float>(body, 468) = 0.5f;
            at<float>(body, 476) = 1.f;
            inertia(452, (X(four_z) * 4.f + X(four_y) * 8.f) * body_mass * 0.0125f);
            inertia(456, (X(four_z) + four_x) * body_mass * 0.083333335816860198974609375f);
            inertia(460, (X(four_y) + four_x) * body_mass * 0.083333335816860198974609375f);
        } else {
            at<float>(body, 468) = 1.f;
            at<float>(body, 476) = 0.5f;
            inertia(452, (X(four_z) + four_y) * body_mass * 0.083333335816860198974609375f);
            inertia(456, (X(four_z) + four_x) * body_mass * 0.083333335816860198974609375f);
            inertia(460, (X(four_y) * 8.f + X(four_x) * 4.f) * body_mass * 0.0125f);
        }
        inertia(68, X(diameter_x) * diameter_z * diameter_y * 0.5f);
        break;
    case 3:
        at<float>(body, 468) = at<float>(body, 476) = 1.f;
        at<Dword>(body, 472) = 1060437492;
        inertia(452, (X(four_z) + four_y) * body_mass * 0.083333335816860198974609375f);
        inertia(456, X(radius) * radius * body_mass * 0.5f);
        inertia(460, (X(four_y) + four_x) * body_mass * 0.083333335816860198974609375f);
        inertia(68, X(radius) * radius * diameter_y * 3.1415927410125732421875f);
        break;
    case 4: {
        at<Dword>(body, 468) = at<Dword>(body, 472) = at<Dword>(body, 476) = 1060437492;
        const float moment = float(X(x) * body_mass * 0.4f);
        at<float>(body, 452) = at<float>(body, 456) = at<float>(body, 460) = moment;
        inertia(68, X(square_x) * x * 4.18879032135009765625f);
        break;
    }
    default:
        report_body_shape_error(1023);
    }
    inertia(64, X(at<float>(body, 32)) / X(at<float>(body, 68)));
    build_body_vertices(at<int>(body, 136), reinterpret_cast<float *>(bytes(body) + 120),
                        reinterpret_cast<float *>(bytes(body) + 140));
    set_body_material(body, material);
    std::memcpy(bytes(body) + 692, saved::identity_matrix, 36);
    matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    std::memcpy(bytes(body) + 72, saved::identity_matrix, 36);
    at<Dword>(body, 532) = at<Dword>(body, 600) = 0;
    for (unsigned offset = 8; offset <= 28; offset += 4)
        at<Dword>(body, offset) = 0;
    at<Dword>(body, 108) = 0;
    at<float>(body, 112) = 1.f;
    at<Dword>(body, 116) = 0;
    at<std::uint16_t>(body, 596) = 0;
    std::memset(bytes(body) + 604, 0, 72);
    at<Dword>(body, 732) = 0;
    at<Byte>(body, 730) = 0;
    clear_ground_contact(bytes(body) + 508);
    at<Byte>(body, 729) = 1;
    at<Byte>(body, 728) = 1;
}
} // namespace nfs::game::runtime::physics
