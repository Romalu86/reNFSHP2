#include "remote_car_runtime.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::network::snapshots {

void apply_reset_snapshot(Dword target_tick, void *car) {
    using game::runtime::at;
    using game::runtime::bytes;
    auto *queue = car_queue(car);
    Snapshot candidate, selected;
    bool changed = false;
    EnterCriticalSection(&queue->critical);
    while (queue->head) {
        copy_state(&candidate, &queue->head->state);
        if (target_tick && target_tick < candidate.tick && queue->current.tick != 4294967295u &&
            target_tick <= queue->current.tick + 900u)
            break;
        const Dword kind = pop(queue, &candidate);
        if (queue->current.tick == 4294967295u || queue->current.tick <= candidate.tick) {
            queue->position_only = kind;
            changed = true;
            copy_state(&queue->current, &candidate);
            if (!target_tick)
                break;
        }
    }
    if (changed) {
        queue->speed = float(game::runtime::hardware::vector_length(candidate.velocity));
        for (unsigned axis = 0; axis < 3; ++axis)
            queue->velocity[axis] = candidate.velocity[axis];
    }
    Dword kind = queue->position_only;
    copy_state(&selected, &queue->current);
    LeaveCriticalSection(&queue->critical);
    if (!changed) {
        EnterCriticalSection(&queue->critical);
        while (queue->head) {
            peek(queue, &candidate);
            const Dword next_kind = pop(queue, &candidate);
            if (queue->current.tick != 4294967295u && candidate.tick < queue->current.tick)
                continue;
            queue->position_only = next_kind;
            changed = true;
            copy_state(&queue->current, &candidate);
            // The saved test uses the previously retained kind, not next_kind.
            if (!kind) {
                queue->speed = float(game::runtime::hardware::vector_length(candidate.velocity));
                for (unsigned axis = 0; axis < 3; ++axis)
                    queue->velocity[axis] = candidate.velocity[axis];
            }
            break;
        }
        kind = queue->position_only;
        copy_state(&selected, &queue->current);
        LeaveCriticalSection(&queue->critical);
        if (!changed)
            return;
    }
    if (selected.tick == 4294967295u)
        return;
    void *body = at<void *>(car, 1768);
    if (!kind) {
        for (unsigned axis = 0; axis < 3; ++axis)
            at<float>(body, 20 + axis * 4) = selected.angular_velocity[axis];
        for (unsigned axis = 0; axis < 3; ++axis)
            at<float>(body, 8 + axis * 4) = selected.velocity[axis];
        for (unsigned word = 0; word < 9; ++word)
            at<float>(body, 692 + word * 4) = selected.rotation[word];
        game::runtime::matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                                           reinterpret_cast<float *>(bytes(body) + 72));
    } else {
        for (unsigned field = 8; field <= 28; field += 4)
            at<Dword>(body, field) = 0;
    }
    at<float>(body, 680) = selected.position[0];
    at<float>(body, 684) = selected.position[1];
    if (kind) {
        at<Dword>(car, 3160) = 0;
        at<Dword>(car, 3164) = 0;
        at<Dword>(car, 3168) = 0;
    }
    at<float>(body, 688) = selected.position[2];
    at<PositionState>(car, 7940) = PositionState::Warp;
    for (int player = 0; player < at<int>(game::runtime::system(72), 176); ++player) {
        void *director = game::tick_services[player];
        if (director && at<void *>(director, 300) == car) {
            game::world::camera::refresh_target(director);
            const Dword section = selected.flags >> 8 & 63;
            // This function reads the first player's current section for this loop.
            while (at<Dword>(game::runtime::system(36), 48) != section)
                game::runtime::track::select_network_section(game::runtime::system(36), player,
                                                             int(section));
        }
    }
}

int camera_warp_requires_track_load(void *car, const Snapshot *snapshot) {
    using game::runtime::at;
    int viewed = 0;
    if (car) {
        for (int player = 0; player < at<int>(game::runtime::system(72), 176); ++player)
            if (game::tick_services[player] &&
                at<void *>(game::tick_services[player], 300) == car) {
                viewed = 1;
                break;
            }
        float position[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            position[axis] = snapshot->position[axis];
        auto *boundary =
            static_cast<Dword *>(game::runtime::track::boundary(game::runtime::system(36), 1, 0));
        if (game::runtime::track::contains(boundary, position))
            viewed = 0;
    }
    return viewed;
}
} // namespace nfs::network::snapshots
