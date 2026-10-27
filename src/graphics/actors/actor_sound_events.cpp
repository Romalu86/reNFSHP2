#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../animation/animation_resource_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::world::actor_resources {
void *event_actor{};
Dword *(&actor_event_targets)[100] = actor_event_storage.targets;
namespace {
Dword default_event_target[2]{};
extern const Dword event_target_methods[1];
} // namespace

int bounded_listener_distance(const float *listener, const float *position) {
    int distance = runtime::rounded_cell(float(runtime::retained_point_distance(listener, position)));
    return distance < 0 || distance > 10000 ? 10000 : distance;
}

void update_actor_sound(int index, Dword context, const float *position, const float *velocity) {
    using namespace audio::state;
    auto *entry = track_data + audio_layout::actor_sounds - audio_layout::wind_sections + Dword(index) * 36;
    if (at<int>(entry, 4) == -1)
        return;
    Dword record[9];
    std::memcpy(record, entry, 36);
    if (bounded_listener_distance(
            reinterpret_cast<float *>(runtime::race_audio::pending::listener_state),
            position) > int(record[2]))
        return;
    Dword flags;
    if (!record[5])
        flags = 257;
    else {
        if (at<Byte>(record, 28))
            return;
        flags = 513;
        at<Byte>(entry, 28) = 1;
    }
    if (record[6]) {
        float x = velocity[0];
        if (x < 0 || std::isnan(x))
            x = -x;
        if (double(x) <= .2) {
            float z = velocity[2];
            if (z < 0 || std::isnan(z))
                z = -z;
            if (double(z) <= .2)
                return;
        }
    }
    if (record[8])
        flags |= 2048;
    Dword request[12];
    std::memcpy(request, position, 12);
    std::memcpy(request + 3, velocity, 12);
    request[6] = context;
    request[7] = flags;
    for (unsigned word = 0; word < 4; ++word)
        request[8 + word] = record[1 + word];
    runtime::race_audio::pending::update_stream_emitter(request);
}

void submit_actor_sound(void *actor, int event) {
    using namespace audio::state;
    int index = int(Dword(event) - 2000u);
    if (index >= track<int>(audio_layout::actor_sound_event_count))
        return;
    Dword record[9];
    std::memcpy(record, track_data + 824 - audio_layout::wind_sections + Dword(index) * 36, 36);
    Dword flags = (record[5] ? 256u : 0u) + 257u;
    if (record[8])
        flags |= 2048;
    float velocity[3] = {at<float>(actor, 104), at<float>(actor, 108), -at<float>(actor, 112)};
    Dword request[12];
    world::invoke(actor, 0, request);
    std::memcpy(request + 3, velocity, 12);
    request[6] = at<Dword>(actor, 100);
    request[7] = flags;
    for (unsigned word = 0; word < 4; ++word)
        request[8 + word] = record[1 + word];
    runtime::race_audio::pending::update_stream_emitter(request);
}

void dispatch_actor_event(void *actor, Dword identifier, int value) {
    if (value >= 2000 && value < 3000) {
        submit_actor_sound(actor, value);
        return;
    }
    if (auto callback = at<void(__cdecl *)(Dword, int)>(actor, 412))
        callback(identifier, value);
}
// Virtual entry ignores ECX and returns with RET12.
void __fastcall receive_actor_event(void *, void *, Dword identifier, void *event, Dword) {
    if (event_actor) {
        auto format = kernel::numeric::current_format();
        Dword value =
            Dword(kernel::numeric::Extended(at<float>(event, 12), format).truncate_to_integer());
        dispatch_actor_event(event_actor, identifier, int(value));
    }
}
namespace {
const Dword event_target_methods[1] = {reinterpret_cast<Dword>(receive_actor_event)};
}

void initialize_actor_event_target() {
    default_event_target[1] = 0;
    default_event_target[0] = reinterpret_cast<Dword>(event_target_methods);
}

void bind_default_actor_events() {
    int count = animation_events.count;
    for (int index = count - 1; index >= 0; --index)
        actor_event_targets[index] = default_event_target;
    for (int index = 0; index < count; ++index)
        actor_event_targets[index] = default_event_target;
    // The saved sentinel at index100 overlaps the adjacent EventRegistry.
    *reinterpret_cast<Dword **>(reinterpret_cast<Byte *>(&actor_event_storage) + Dword(count) * 4) =
        default_event_target;
}
} // namespace nfs::game::world::actor_resources
NFS_KERNEL_INITIALIZER(startup_initialize_actor_event_target, ".CRT$XCU_NFS_00690008",
                       nfs::game::world::actor_resources::initialize_actor_event_target);
