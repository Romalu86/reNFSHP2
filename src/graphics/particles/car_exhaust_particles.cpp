#include "../../math/vectors/retained_vector_providers.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../vehicles/rendering/vehicle_attachment_transforms.hpp"
#include <cstring>
#include <cmath>
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Exhaust particle placement requires native retained intermediates");
namespace nfs::game::world::camera {
void active_camera_position(void *, float *);
}
namespace nfs::game::runtime::physics::contacts_pending {
long double accumulated_body_damage(void *);

void emit_exhaust_particle(void *car, const float *position, float amount) {
    if (!audio::vehicle::car_audio_suppressed(car)) {
        void *body = at<void *>(car, 1768);
        float closest = -1;
        const int cameras = split_screen() ? 2 : 1;
        for (int index = 0; index < cameras; ++index) {
            float camera[3];
            world::camera::active_camera_position(game::tick_services[index], camera);
            long double distance;
            hardware::store_point_distance(camera, &at<float>(body, 680), &distance);
            if (distance < static_cast<long double>(closest) || closest < 0)
                closest = float(distance);
        }
        if (closest > 150)
            return;
    }
    if (amount < 0)
        amount = 0;
    else if (amount > 1)
        amount = 1;
    void *body = at<void *>(car, 1768);
    float direction[3] = {float(-static_cast<long double>(at<float>(body, 716))),
                           at<float>(body, 720), at<float>(body, 724)};
    float velocity[3] = {at<float>(body, 8), at<float>(body, 12),
                          float(-static_cast<long double>(at<float>(body, 16)))};
    void *definition = particles::lifecycle::particle_prefix;
    while (at<Dword>(definition, 4) && at<Dword>(definition, 4) != 13)
        definition = bytes(definition) + 152;
    if (!at<Dword>(definition, 4))
        definition = nullptr;
    particles::lifecycle::emit_car_particle(definition, position, amount, direction,
                                            saved::zero_vector, velocity, 1, 1);
}

void update_exhaust_effects(void *car) {
    auto retain = [](float value) { return static_cast<long double>(value); };
    const long double damage = accumulated_body_damage(bytes(car) + 7952);
    if (damage < .25f)
        return;
    const float amount = float((damage - .25f) * 1.33333337306976318359375f);
    Dword attachments[2][9];
    float points[2][3];
    int count = 0;
    if (world::vehicle_render::read_vehicle_attachment(at<void *>(car, 3372), 19, attachments[0],
                                                       reinterpret_cast<Dword *>(points[0])))
        count = 1;
    else {
        points[0][0] = .8f;
        points[0][1] = -.5f;
        points[0][2] = 3;
        std::memcpy(attachments[0], saved::identity_matrix, 36);
    }
    if (world::vehicle_render::read_vehicle_attachment(at<void *>(car, 3372), 20, attachments[1],
                                                       reinterpret_cast<Dword *>(points[1])))
        ++count;
    else {
        points[1][0] = -.8f;
        points[1][1] = -.5f;
        points[1][2] = 3;
        std::memcpy(attachments[1], saved::identity_matrix, 36);
    }
    // The loop uses the first count entries, including the first fallback when only attachment14
    // exists. It does not compact the successful attachments.
    for (int index = 0; index < count; ++index) {
        void *body = at<void *>(car, 1768);
        float origin[3] = {at<float>(body, 680), at<float>(body, 684),
                            float(-retain(at<float>(body, 688)))};
        float matrix[9] = {
            at<float>(body, 692), at<float>(body, 696), float(-retain(at<float>(body, 700))),
            at<float>(body, 704), at<float>(body, 708), float(-retain(at<float>(body, 712))),
            float(-retain(at<float>(body, 716))), float(-retain(at<float>(body, 720))),
            at<float>(body, 724)};
        float rotated[3] = {0, 0, 0};
        matrices::transform_vectors(1, points[index], matrix, rotated);
        float position[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = float(retain(rotated[axis]) + origin[axis]);
        emit_exhaust_particle(car, position, amount);
    }
}
} // namespace nfs::game::runtime::physics::contacts_pending
