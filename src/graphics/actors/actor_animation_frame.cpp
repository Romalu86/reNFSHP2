#include "../animation/animation_resource_runtime.hpp"
#include "nfs/graphics/display/graphics_error.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>
namespace nfs::game::world::vehicle_render {
void sample_actor_root(void *, Dword *, float *, Dword *, Dword *, void *);
}
namespace nfs::game::world::actor_resources {

void evaluate_actor_channel(void *owner, Dword index, Dword *pose) {
    vehicle_render::evaluate_animation_channel(offset(at<void *>(owner, 12), index * 20), pose);
}

Byte advance_animation_clock(void *owner) {
    if (at<Byte>(owner, 8)) {
        at<Dword>(owner, 4) = 0;
        return 0;
    }
    Dword now = graphics::unresolved::camera_state;
    at<Dword>(owner, 4) = now - at<Dword>(owner, 0);
    if (!at<Dword>(owner, 4))
        return 0;
    const int channel_count = at<int>(owner, 16);
    at<Dword>(owner, 0) = now;
    Byte wrapped = 0;
    if (channel_count > 0) {
        void *channel = at<void *>(owner, 12);
        const Dword elapsed_ticks = at<Dword>(owner, 4);
        for (int index = 0; index < channel_count; ++index, channel = offset(channel, 20)) {
            if (elapsed_ticks && at<Dword>(channel, 16)) {
                const auto elapsed = static_cast<long double>(std::int32_t(elapsed_ticks));
                at<Dword>(channel, 8) = at<Dword>(channel, 4);
                const auto next = elapsed * at<float>(channel, 0) + at<float>(channel, 4);
                at<volatile float>(channel, 4) = float(next);
                // Compare the retained value, not the rounded frame just stored.
                if (next > static_cast<long double>(at<float>(channel, 12))) {
                    at<Dword>(channel, 4) = 0;
                    wrapped = 1;
                }
            }
        }
    }
    at<Byte>(owner, 10) = wrapped;
    return wrapped;
}

Byte update_actor_animation(void *actor) {
    if (!at<Byte>(actor, 84))
        return 0;
    void *owner = offset(actor, 52);
    Byte wrapped = advance_animation_clock(owner);
    if (!at<Dword>(actor, 56))
        return wrapped;
    if (wrapped)
        if (auto callback = at<ActorAnimationCallback>(actor, 400))
            callback(actor, wrapped);
    const Byte root_locked = at<volatile Byte>(actor, 372);
    const Dword flags = at<volatile Dword>(actor, 268);
    if (!root_locked) {
        void *channels = at<void *>(actor, 64);
        auto *displacement = reinterpret_cast<float *>(offset(actor, 104));
        bool scale = true;
        if (flags & 1) {
            if (at<Dword>(channels, 36))
                vehicle_render::sample_actor_root(actor, vehicle_render::animation_pose_scratch(),
                                                  displacement, nullptr,
                                                  reinterpret_cast<Dword *>(offset(actor, 116)),
                                                  offset(channels, 20));
        } else if (flags & 2) {
            if (at<Dword>(channels, 36))
                vehicle_render::sample_actor_root(actor, vehicle_render::animation_pose_scratch(),
                                                  displacement,
                                                  reinterpret_cast<Dword *>(offset(actor, 124)),
                                                  nullptr, offset(channels, 20));
            scale = false;
        } else {
            Dword *pose = vehicle_render::animation_pose_scratch();
            if (at<Dword>(channels, 36)) {
                at<volatile Dword>(pose, 40) = 0;
                at<volatile Dword>(pose, 36) = 0;
                at<volatile Dword>(pose, 32) = 0;
                vehicle_render::evaluate_animation_channel(offset(channels, 20), pose);
                const Dword x = pose[8], y = pose[9], z = pose[10];
                at<volatile Dword>(displacement, 0) = x;
                at<volatile Dword>(displacement, 4) = y;
                at<volatile Dword>(displacement, 8) = z;
                at<volatile Dword>(pose, 40) = 0;
                at<volatile Dword>(pose, 36) = 0;
                at<volatile Dword>(pose, 32) = 0;
            }
            at<Dword>(actor, 116) = at<Dword>(actor, 108);
        }
        if (scale) {
            const float factor = at<float>(actor, 120);
            for (unsigned axis = 0; axis < 3; ++axis)
                displacement[axis] = float(static_cast<long double>(factor) * displacement[axis]);
            at<float>(actor, 116) =
                float(static_cast<long double>(at<float>(actor, 116)) *
                      at<float>(actor, 120));
        }
    }
    event_actor = actor;
    evaluate_actor_channel(owner, 2, reinterpret_cast<Dword *>(actor_event_targets));
    return wrapped;
}

void actor_world_position(void *actor, float *output) {
    if (!(at<Byte>(actor, 268) & 3)) {
        float displacement[3];
        actor_displacement(actor, displacement);
        for (unsigned axis = 0; axis < 3; ++axis) {
            const volatile float position = float(static_cast<long double>(displacement[axis]) +
                                                  at<float>(actor, 124 + axis * 4));
            output[axis] = position;
        }
    } else {
        const Dword x = at<Dword>(actor, 124), y = at<Dword>(actor, 128);
        at<volatile Dword>(output, 0) = x;
        const Dword z = at<Dword>(actor, 132);
        at<volatile Dword>(output, 4) = y;
        at<volatile Dword>(output, 8) = z;
    }
}

void retire_finished_actors(void *system) {
    void *list = at<void *>(system, 92);
    bool retired = false;
    for (int index = 0; index < (list ? at<int>(list, 12) : 0); ++index) {
        void *actor = list ? world::actor_at(list, index) : nullptr;
        if (!at<Byte>(actor, 61)) {
            world::reset_actor_render_state(offset(actor, 52));
            retired = true;
        }
    }
    if (!retired || !at<Byte>(system, 104))
        return;
    if (list)
        world::clear_actor_list(list);
    if (void *table = at<void *>(system, 108)) {
        world::invoke(table, 0, 1u);
        at<Dword>(system, 108) = 0;
    }
    int count = at<int>(system, 84);
    if (count > 0) {
        auto **paths = at<void **>(system, 88);
        for (int index = 0; index < count; ++index)
            gui::application_release(paths[index]);
    }
    if (count)
        gui::application_release(at<void *>(system, 88));
    at<Dword>(system, 84) = 0;
    world::release_actor_list_base();
    world::create_character_table(system);
    world::pending::release_vehicle_character_bindings_entry(subsystem(52));
    at<world::CharacterSequenceCallback>(system, 96)(at<void *>(system, 100));
    const auto ticks = static_cast<long double>(std::int32_t(graphics::unresolved::camera_state));
    at<world::CharacterSequenceCallback>(system, 96) = world::character_frame_callback;
    const auto elapsed = ticks * 0.01666666753590106964111328125f;
    at<volatile Byte>(system, 104) = 0;
    static_cast<volatile float &>(frame_clock::elapsed) = float(elapsed);
}

void __fastcall pending::advance_actor(void *actor, void *) {
    auto local = [] {
        return runtime::physics::local_car(static_cast<Dword *>(subsystem(68)), 0);
    };
    if (local() && at<void *>(local(), 1768)) {
        void *body = at<void *>(local(), 1768);
        float position[3];
        std::memcpy(position, offset(body, 680), 8);
        position[2] = -at<float>(body, 688);
        float actor_position[3];
        actor_world_position(actor, actor_position);
        const float radius = at<float>(actor, 176);
        const auto distance = runtime::hardware::distance_squared(position, actor_position);
        if (!(static_cast<long double>(radius) * radius > distance))
            return;
    }
    if (at<Dword>(actor, 384) && update_actor_animation(actor) && !at<Byte>(actor, 61))
        retire_finished_actors(subsystem(48));
    if (at<Dword>(actor, 56))
        if (auto callback = at<ActorFrameCallback>(actor, 404))
            callback(actor);
    if (at<int>(actor, 96) != -1) {
        float displacement[3];
        std::memcpy(displacement, offset(actor, 104), sizeof(displacement));
        displacement[2] = float(static_cast<long double>(displacement[2]) * -1.f);
        float position[3];
        world::invoke(actor, 0, position);
        const Dword sound_context = at<Dword>(actor, 100);
        const int sound = at<int>(actor, 96);
        update_actor_sound(sound, sound_context, position, displacement);
    }
}
} // namespace nfs::game::world::actor_resources
