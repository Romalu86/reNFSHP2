#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "../../ai/driving/ai_car_behavior.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <intrin.h>
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::physics::contacts_pending {
const Dword body_contact_friction[2] = {1056964608, 1054615798};
Dword body_contact_flags{};
} // namespace nfs::game::runtime::physics::contacts_pending
namespace nfs::game::runtime::physics {

void predict_body_pose(float seconds, const float *velocity, const float *angular, float *position,
                       float *rotation, const float *centre) {
    auto E = [](float value) { return static_cast<long double>(value); };
    float previous[3], next[3];
    matrices::transform_positions(1, centre, rotation, position, previous, 12);
    const float rotation_z = float(E(seconds) * angular[2]);
    const float rotation_y = float(E(seconds) * angular[1]);
    const float rotation_x = float(E(seconds) * angular[0]);
    matrices::rotate(rotation, rotation_x, rotation_y, rotation_z, rotation);
    matrices::transform_positions(1, centre, rotation, position, next, 12);
    for (unsigned axis = 0; axis < 3; ++axis)
        position[axis] =
            float(((E(seconds) * velocity[axis] + previous[axis]) - next[axis]) + position[axis]);
}

void submit_body_impact(void *body) {
    auto E = [](float value) { return static_cast<long double>(value); };
    const float denominator = at<float>(body, 660);
    const auto force = denominator != 0 ? E(at<float>(body, 608)) / E(denominator)
                                         : E(at<float>(body, 608)) * at<float>(body, 36);
    pending::submit_impact_entry(reinterpret_cast<float *>(bytes(body) + 640),
                                    at<Dword>(body, 40), 20, float(force * .5f), 0);
}

void update_contacts(void *body, Byte allow_remote) {
    auto E = [](float value) { return static_cast<long double>(value); };
    Dword excluded = Dword(-1);
    void *car = vehicle_car(body);
    if (!car)
        allow_remote = 1;
    else {
        void *vehicle = at<void *>(car, 7988);
        if (game::audio::vehicle::car_audio_suppressed(car))
            excluded = at<Dword>(car, 1780);
        if (!vehicle ||
            legacy::local_player_near_150(invoke<const float *>(vehicle, 0)))
            allow_remote = 1;
    }
    float predicted_rotation[9], predicted_position[3];
    __movsd(reinterpret_cast<unsigned long *>(predicted_rotation),
            reinterpret_cast<const unsigned long *>(bytes(body) + 692), 9);
    __movsd(reinterpret_cast<unsigned long *>(predicted_position),
            reinterpret_cast<const unsigned long *>(bytes(body) + 680), 3);
    Dword surface = 0;
    if (void *polygon = at<void *>(body, 532))
        surface = at<Byte>(polygon, 59) & 15;
    predict_body_pose(0.0333333350718021392822265625f, reinterpret_cast<float *>(bytes(body) + 8),
                      reinterpret_cast<float *>(bytes(body) + 20), predicted_position,
                      predicted_rotation, reinterpret_cast<float *>(bytes(body) + 44));
    float vertices[78];
    int count;
    const bool detached = at<std::int16_t>(body, 596) == 0 &&
                          std::fabs(at<float>(body, 112)) > 0.707f && !is_local_vehicle(body);
    if (detached) {
        constexpr unsigned selected[] = {101, 59, 104, 68, 107, 62, 110, 65};
        for (unsigned vertex = 0; vertex < 8; ++vertex)
            __movsd(reinterpret_cast<unsigned long *>(vertices + vertex * 3),
                    reinterpret_cast<const unsigned long *>(static_cast<Dword *>(body) + selected[vertex]), 3);
        count = 8;
    } else if (!vehicle_car(body)) {
        __movsd(reinterpret_cast<unsigned long *>(vertices),
                reinterpret_cast<const unsigned long *>(bytes(body) + 140), 24);
        count = 8;
    } else {
        __movsd(reinterpret_cast<unsigned long *>(vertices),
                reinterpret_cast<const unsigned long *>(bytes(body) + 140), 78);
        count = 26;
    }
    for (int vertex = 0; vertex < count; ++vertex) {
        const float *local = vertices + vertex * 3;
        float current[3], previous[3], contact_point[3], contact_normal[3];
        matrices::transform_positions(1, local, reinterpret_cast<float *>(bytes(body) + 692),
                                      reinterpret_cast<float *>(bytes(body) + 680), current, 12);
        matrices::transform_positions(1, local, predicted_rotation, predicted_position, previous,
                                      12);
        Dword maximum_height_bits;
        const int kind = contacts_pending::find_body_edge_contact(
            current, previous, contact_normal, contact_point, &maximum_height_bits, excluded,
            surface);
        if (!kind)
            continue;
        const auto dz = E(current[2]) - previous[2];
        const auto dx = E(current[0]) - previous[0];
        const float squared = float(dx * dx + dz * dz);
        const float travel = float(__builtin_sqrtl(E(squared)));
        contact_point[1] = current[1];
        if ((kind != 255 || allow_remote) &&
            contacts_pending::apply_body_contact(body, contact_normal, contact_point, travel,
                                                 contacts_pending::body_contact_friction[0],
                                                 contacts_pending::body_contact_friction[1],
                                                 maximum_height_bits,
                                                 contacts_pending::body_contact_flags)) {
            submit_body_impact(body);
            __movsd(reinterpret_cast<unsigned long *>(predicted_rotation),
            reinterpret_cast<const unsigned long *>(bytes(body) + 692), 9);
            __movsd(reinterpret_cast<unsigned long *>(predicted_position),
            reinterpret_cast<const unsigned long *>(bytes(body) + 680), 3);
            predict_body_pose(0.0333333350718021392822265625f, reinterpret_cast<float *>(bytes(body) + 8),
                              reinterpret_cast<float *>(bytes(body) + 20), predicted_position,
                              predicted_rotation, reinterpret_cast<float *>(bytes(body) + 44));
        }
    }
}
} // namespace nfs::game::runtime::physics
