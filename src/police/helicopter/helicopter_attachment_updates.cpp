#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "../../vehicles/rendering/vehicle_attachment_transforms.hpp"
#include <cmath>
#include <cstdlib>
namespace nfs::game::runtime::ai::lifecycle::pending {
 
void release_front_attachment_offset() {
}
 
void release_rear_attachment_offset() {
}
namespace {
void update_attachment(void *helicopter, void *attachment, unsigned offset_address, bool rear) {
    if (!attachment)
        return;
    void *target_body = at<void *>(attachment, 80);
    void *body = at<void *>(at<void *>(helicopter, 368), 80);
    const auto *rotation = reinterpret_cast<const float *>(bytes(body) + 692);
    float position[3] = {at<float>(body, 680), at<float>(body, 684), at<float>(body, 688)};
    Dword compact_rotation[9], compact_position[3];
    for (unsigned index = 0; index < 9; ++index)
        compact_rotation[index] = at<Dword>(body, 692 + index * 4);
    for (unsigned index = 0; index < 3; ++index)
        compact_position[index] = at<Dword>(body, 680 + index * 4);
    graphics::math::Matrix local;
    const float radians =
        float(static_cast<long double>(at<float>(helicopter, 1656)) * 0.01745329238474369049072265625f);
    if (std::isnan(radians) || radians == 0) {
        graphics::math::diagonal(local, 1, 1, 1, 1);
    } else {
        // This is the inlined axis(0,1,0) path, including products by zero.
        const long double sine = std::sin(static_cast<long double>(radians));
        const long double cosine = std::cos(static_cast<long double>(radians));
        const long double x = 0.L, y = 1.L, z = 0.L, complement = 1.L - cosine;
        const long double product_x = x * complement;
        const float product_y = float(complement * y), product_z = float(complement * z);
        const float sine_x = float(sine * x), sine_y = float(sine * y), sine_z = float(sine * z);
        local[0] = float(x * product_x + cosine);
        local[1] = float(y * product_x + sine_z);
        local[2] = float(product_x * z - sine_y);
        local[3] = 0;
        local[4] = float(static_cast<long double>(product_y) * x - sine_z);
        local[5] = float(static_cast<long double>(product_y) * y + cosine);
        local[6] = float(static_cast<long double>(product_y) * z + sine_x);
        local[7] = 0;
        local[8] = float(static_cast<long double>(product_z) * x + sine_y);
        local[9] = float(static_cast<long double>(product_z) * y - sine_x);
        local[10] = float(static_cast<long double>(product_z) * z + cosine);
        local[11] = 0;
        local[12] = local[13] = local[14] = 0;
        local[15] = 1;
    }
    if (rear)
        graphics::math::append_axis_rotation(local, 90, 0, 0, 1);
    graphics::math::append_translation(local, state<float>(offset_address),
                                       state<float>(offset_address + 4),
                                       state<float>(offset_address + 8));
    graphics::math::Matrix parent, combined;
    graphics::math::from_compact_transform(parent, rotation, position);
    graphics::math::multiply(local, parent, combined);
    kernel::memory_copy(parent, combined, 64);
    world::vehicle_render::read_model_transform(reinterpret_cast<const Dword *>(parent),
                                                compact_rotation, compact_position);
    for (unsigned index = 0; index < 3; ++index)
        at<Dword>(target_body, 680 + index * 4) = compact_position[index];
    for (unsigned index = 0; index < 9; ++index)
        at<Dword>(target_body, 692 + index * 4) = compact_rotation[index];
    matrices::transpose(reinterpret_cast<const float *>(bytes(target_body) + 692),
                        reinterpret_cast<float *>(bytes(target_body) + 72));
    synchronize_track_vehicle(attachment);
}
} // namespace

void update_helicopter_attachments(void *helicopter) {
    void *attachment = at<void *>(helicopter, 372);
    if (!(state<Dword>(7352816) & 1)) {
        state<Dword>(7352816) |= 1;
        for (unsigned index = 0; index < 3; ++index)
            state<Dword>(7352804 + index * 4) = state<Dword>(7352840 + index * 4);
        std::atexit(release_front_attachment_offset);
    }
    update_attachment(helicopter, attachment, 7352804, false);
}

void update_helicopter_rendering(void *helicopter) {
    void *attachment = at<void *>(helicopter, 376);
    if (!(state<Dword>(7352800) & 1)) {
        state<Dword>(7352800) |= 1;
        for (unsigned index = 0; index < 3; ++index)
            state<Dword>(7352788 + index * 4) = state<Dword>(7352852 + index * 4);
        std::atexit(release_rear_attachment_offset);
    }
    update_attachment(helicopter, attachment, 7352788, true);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
