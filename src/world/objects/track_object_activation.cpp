#include "../../physics/bodies/shape_body_runtime.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cstring>
#include <cmath>
namespace nfs::game::world::dependency {
Byte project_boom_to_ground(const float *, Dword *, float *, float *);

long double closest_body_to_boom(void *owner) {
    using namespace runtime::physics::pending;
    float closest = 300;
    for (int index = 0; index <= last_vehicle; ++index) {
        Byte *body = vehicle_states + Dword(index) * 736;
        if (vehicle_flags[index] || !at<Byte>(body, 728) ||
            at<void *>(owner, 80) == body || at<Dword>(body, 732) ||
            at<float>(body, 60) != 0 || std::isnan(at<float>(body, 60)) || at<Byte>(body, 5))
            continue;
        float position[] = {at<float>(owner, 28), at<float>(owner, 32),
                            -at<float>(owner, 36)};
        void *material = at<void *>(at<void *>(at<void *>(owner, 76), 32), 0);
        const auto distance =
            runtime::retained_point_distance(position, reinterpret_cast<float *>(body + 680));
        const auto format = kernel::numeric::current_format();
        auto gap =
            distance -
            (kernel::numeric::Extended(at<float>(material, 68), format) + at<float>(body, 132));
        if (gap < kernel::numeric::Extended(closest, format))
            closest = float(gap);
    }
    return closest;
}

void update_boom_emitter(int index, Dword serial, const float *position) {
    if (index < 0 || index >= 30)
        return;
    Byte *record = services::pending::boom_sound_definitions + Dword(index) * 312;
    int patch = at<int>(record, 296);
    if (patch == -1)
        return;
    Dword parameters[12];
    std::memcpy(parameters, position, 12);
    parameters[3] = parameters[4] = parameters[5] = 0;
    parameters[6] = serial;
    Dword variations = at<Dword>(record, 300);
    if (variations) {
        parameters[7] = 513;
        parameters[8] = network::rules::random() % variations + Dword(patch);
    } else {
        parameters[7] = 257;
        parameters[8] = Dword(patch);
    }
    parameters[11] = 0;
    parameters[9] = at<Dword>(record, 304);
    parameters[10] = at<Dword>(record, 308);
    runtime::race_audio::pending::update_stream_emitter(parameters);
}
namespace pending {

void advance_boom(void *owner) {
    void *definition = at<void *>(owner, 76);
    if (at<std::int16_t>(definition, 28) == 1)
        return;
    float position[3];
    if (at<int>(owner, 104) != -1) {
        world::invoke(owner, 0, position);
        update_boom_emitter(at<int>(owner, 104), at<Dword>(owner, 16), position);
    }
    if (at<Byte>(owner, 96)) {
        at<Dword>(owner, 92) = 0;
        float *location = reinterpret_cast<float *>(offset(owner, 28));
        if (project_boom_to_ground(location, &at<Dword>(owner, 92), location, position)) {
            void *material = at<void *>(at<void *>(at<void *>(owner, 76), 32), 0);
            at<Byte>(owner, 96) = 0;
            at<float>(owner, 32) =
                float(static_cast<long double>(at<float>(material, 60)) + at<float>(owner, 32));
        }
        return;
    }
    if (at<void *>(owner, 80) || !(closest_body_to_boom(owner) <= 3.576240062713623046875f))
        return;
    void *body = runtime::physics::reserve_vehicle();
    at<void *>(owner, 80) = body;
    if (!body)
        return;
    if (!runtime::physics::initialize_shape_body(body, owner, 0)) {
        runtime::physics::release_vehicle(at<void *>(owner, 80));
        return;
    }
    if (at<Byte>(owner, 116))
        at<Dword>(at<void *>(owner, 80), 60) = 0;

    void *material = world::invoke<void *>(owner, 4);
    if (!_stricmp(reinterpret_cast<const char *>(offset(material, 80)), "sign")) {
        body = at<void *>(owner, 80);
        at<float>(body, 48) = float(static_cast<long double>(at<float>(body, 124)) * .5L);
    }
}
} // namespace pending
} // namespace nfs::game::world::dependency
