#include "vehicle_renderable_resources.hpp"
#include "vehicle_attachment_transforms.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::world::vehicle_render {

void construct_animation_channel(void *channel) {
    at<Dword>(channel, 0) = 1051372191;
    at<Dword>(channel, 12) = at<Dword>(channel, 8) = at<Dword>(channel, 4) = at<Dword>(channel, 16) = 0;
}

void allocate_animation_channels(void *owner, const char *label, Dword count, Dword pool) {
    at<Dword>(owner, 16) = count;
    auto *allocation = static_cast<Dword *>(lifecycle::allocate(count * 20 + 4, label, pool));
    if (!allocation) {
        at<Dword>(owner, 12) = 0;
        return;
    }
    *allocation = count;
    try {
        lifecycle::construction::construct_array(
            allocation + 1, 20, int(count), construct_animation_channel, destroy_animation_channel);
    } catch (...) {
        kernel::memory_free(allocation);
        throw;
    }
    at<Dword *>(owner, 12) = allocation + 1;
}

void *find_animation_entry(void *table, Dword identifier) {
    void *entry = at<void *>(table, 4);
    while (entry && at<Dword>(entry, 28) != identifier)
        entry = at<void *>(entry, 12);
    return entry;
}

void bind_animation_channel(void *owner, Dword index, void *animation, Dword rate, Dword time) {
    void *channel = offset(at<void *>(owner, 12), index * 20);
    at<Dword>(channel, 0) = rate;
    at<void *>(channel, 16) = animation;
    at<Dword>(channel, 8) = at<Dword>(channel, 4) = time;
    if (animation)
        world::invoke(animation, 4, offset(channel, 12));
    else
        at<Dword>(channel, 12) = 0;
}

void bind_animation_channel_rotation(void *owner, Dword index, void *animation,
                                     void *duration_source, Dword rate, Dword time) {
    void *channel = offset(at<void *>(owner, 12), index * 20);
    at<void *>(channel, 16) = animation;
    at<Dword>(channel, 0) = rate;
    at<Dword>(channel, 4) = at<Dword>(channel, 8) = time;
    if (duration_source)
        world::invoke(duration_source, 4, offset(channel, 12));
    else
        at<Dword>(channel, 12) = 0;
}

void evaluate_animation_channel(void *channel, Dword *pose) {
    if (void *animation = at<void *>(channel, 16))
        world::invoke(animation, 3, at<Dword>(channel, 8), at<Dword>(channel, 4), pose);
}

void sample_animation_channel(void *channel, Dword *pose, Dword time) {
    if (void *animation = at<void *>(channel, 16))
        world::invoke(animation, 3, time, time, pose);
}

void sample_actor_root(void *actor, Dword *pose, float *displacement, Dword *position, Dword *height,
                       void *channel) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    float previous = at<float>(channel, 4), current = at<float>(channel, 8), first = previous;
    if (previous < current) {
        first = current;
        current =
            float(X(current) - kernel::numeric::Extended::integer(at<int>(actor, 56), format) *
                                   at<float>(channel, 0));
    }
    pose[10] = pose[9] = pose[8] = 0;
    evaluate_animation_channel(channel, pose);
    if (height)
        *height = pose[9];
    if (at<float>(channel, 4) != first || std::isnan(first) || std::isnan(at<float>(channel, 4))) {
        Dword bits;
        std::memcpy(&bits, &first, 4);
        sample_animation_channel(channel, pose, bits);
    }
    if (position)
        std::memcpy(position, pose + 8, 12);
    if (!at<Byte>(actor, 60)) {
        float x, z;
        std::memcpy(&x, pose + 8, 4);
        std::memcpy(&z, pose + 10, 4);
        Dword bits;
        std::memcpy(&bits, &current, 4);
        sample_animation_channel(channel, pose, bits);
        float sampled_x, sampled_z;
        std::memcpy(&sampled_x, pose + 8, 4);
        std::memcpy(&sampled_z, pose + 10, 4);
        displacement[0] = float(X(x) - sampled_x);
        displacement[2] = float(X(z) - sampled_z);
    } else
        displacement[0] = displacement[2] = 0;
    pose[10] = pose[9] = pose[8] = 0;
    displacement[1] = 0;
}

void pending::select_character_animation(void *actor, Dword identifier, Dword time) {
    void *rotation = nullptr, *translation = nullptr, *extra = nullptr;
    if (identifier) {
        void *table = at<void *>(actor, 384);
        rotation = at<void *>(find_animation_entry(table, identifier), 24);
        void *entry = find_animation_entry(table, identifier + 1000000);
        if (entry)
            translation = at<void *>(entry, 24);
        entry = find_animation_entry(table, identifier + 2000000);
        if (entry)
            extra = at<void *>(entry, 24);
        if (!translation)
            translation = rotation;
    }
    void *channels = offset(actor, 52);
    bind_animation_channel(channels, 0, rotation, at<Dword>(actor, 376), time);
    bind_animation_channel(channels, 1, translation, at<Dword>(actor, 376), time);
    bind_animation_channel_rotation(channels, 2, extra, rotation, at<Dword>(actor, 376), time);
    at<Dword>(actor, 92) = identifier;
    if (at<Byte>(actor, 372))
        return;
    void *channel = at<void *>(actor, 64);
    Dword flags = at<Dword>(actor, 268);
    if (flags & 1) {
        if (at<Dword>(channel, 36))
            sample_actor_root(actor, animation_pose_scratch(),
                              reinterpret_cast<float *>(offset(actor, 104)), nullptr,
                              reinterpret_cast<Dword *>(offset(actor, 116)), offset(channel, 20));
    } else if (flags & 2) {
        if (at<Dword>(channel, 36))
            sample_actor_root(
                actor, animation_pose_scratch(), reinterpret_cast<float *>(offset(actor, 104)),
                reinterpret_cast<Dword *>(offset(actor, 124)), nullptr, offset(channel, 20));
    } else {
        Dword *pose = animation_pose_scratch();
        if (at<Dword>(channel, 36)) {
            pose[10] = pose[9] = pose[8] = 0;
            evaluate_animation_channel(offset(channel, 20), pose);
            std::memcpy(offset(actor, 104), pose + 8, 12);
            pose[10] = pose[9] = pose[8] = 0;
        }
        at<Dword>(actor, 116) = at<Dword>(actor, 108);
    }
}
// Fourth saved stack word is ignored, RET16.
void pending::bind_driver(void *owner, Dword cop, Byte left_side, void *attachments, Dword) {
    at<Byte>(owner, 48) = left_side;
    at<Dword>(owner, 56) = cop;
    at<void *>(owner, 20) = attachments;
    if (!attachment_enabled(attachments, 40))
        return;
    Dword rotation[9];
    read_attachment_transform(attachments, 40, rotation, static_cast<Dword *>(owner));
    auto *rules = static_cast<Dword *>(subsystem(72));
    if (network::packets::rule_value(rules, 6) == 2 || network::packets::rule_value(rules, 6) == 3)
        at<float>(owner, 0) = -at<float>(owner, 0);
    initialize_driver_actor(owner);
}
} // namespace nfs::game::world::vehicle_render
