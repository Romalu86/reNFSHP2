#include "remote_car_runtime.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cmath>
namespace nfs::application::pending {
std::uint16_t lookup_player_identifier(void *, kernel::Dword);
void *race_participant_owner_view(void *, kernel::Dword);
} // namespace nfs::application::pending
namespace nfs::network {
Dword race_pending(Dword *);
namespace snapshots {
void send_ghost_period(void *, std::uint16_t, std::uint8_t);
}
} // namespace nfs::network
namespace nfs::services::pending {
using namespace game::runtime;
using network::snapshots::PositionState;
using network::snapshots::Snapshot;
namespace {
long double X(float value) {
    return static_cast<long double>(value);
}
void warp_to_snapshot(void *car, const Snapshot &snapshot) {
    void *body = at<void *>(car, 1768);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(body, 20 + axis * 4) = snapshot.angular_velocity[axis];
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(body, 8 + axis * 4) = snapshot.velocity[axis];
    for (unsigned word = 0; word < 9; ++word)
        at<float>(body, 692 + word * 4) = snapshot.rotation[word];
    matrices::transpose(reinterpret_cast<float *>(bytes(body) + 692),
                        reinterpret_cast<float *>(bytes(body) + 72));
    body = at<void *>(car, 1768);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(body, 680 + axis * 4) = snapshot.position[axis];
    if (body)
        at<Byte>(at<void *>(car, 1768), 728) = 1;
}
} // namespace
// The saved ECX receiver is unused.
void update_remote_car(void *, Dword target_tick, void *car) {
    namespace snapshots = network::snapshots;
    if (!race_started) {
        at<PositionState>(car, 7940) = PositionState::Drone;
        at<Dword>(car, 7944) = network::transport_time();
        return;
    }
    auto *queue = snapshots::car_queue(car);
    Snapshot candidate, selected;
    bool changed = false;
    EnterCriticalSection(&queue->critical);
    while (queue->head) {
        snapshots::copy_state(&candidate, &queue->head->state);
        if (target_tick && target_tick < candidate.tick && queue->current.tick != 4294967295u &&
            target_tick <= queue->current.tick + 900u)
            break;
        const Dword kind = snapshots::pop(queue, &candidate);
        if (queue->current.tick == 4294967295u || queue->current.tick <= candidate.tick) {
            queue->position_only = kind;
            changed = true;
            snapshots::copy_state(&queue->current, &candidate);
            if (!target_tick)
                break;
        }
    }
    if (changed) {
        queue->speed = float(hardware::vector_length(candidate.velocity));
        for (unsigned axis = 0; axis < 3; ++axis)
            queue->velocity[axis] = candidate.velocity[axis];
    }
    snapshots::copy_state(&selected, &queue->current);
    const Dword position_only = queue->position_only;
    LeaveCriticalSection(&queue->critical);
    if (selected.tick == 4294967295u) {
        at<PositionState>(car, 7940) = PositionState::Drone;
        return;
    }
    const Dword latest_tick = at<volatile Dword>(car, 7928),
               latest_game_tick = at<Dword>(system(104), 80);
    at<Dword>(system(104), 80) = latest_game_tick < latest_tick ? latest_tick : latest_game_tick;
    const std::int32_t tick_delta = static_cast<std::int32_t>(target_tick - selected.tick);
    const bool future_snapshot = tick_delta < 0;
    const auto retained_delta = static_cast<long double>(tick_delta);
    const float stored_delta = float(retained_delta);
    const float time_gap = std::fabs(float(X(stored_delta) * 0.01666666753590106964111328125f));
    const auto prediction_seconds = future_snapshot ? X(0.f) : retained_delta * 0.01666666753590106964111328125f;
    float trajectory[3], error[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        trajectory[axis] = float(X(selected.velocity[axis]) * prediction_seconds);
    void *body = at<void *>(car, 1768);
    error[0] = float(X(selected.position[0]) + trajectory[0] - at<float>(body, 680));
    error[1] = float(X(selected.position[1]) + trajectory[1] - at<float>(body, 684));
    const float predicted_z = float(X(selected.position[2]) + trajectory[2]);
    error[2] = float(X(predicted_z) - at<float>(body, 688));
    for (unsigned axis = 0; axis < 3; ++axis)
        queue->prediction_error[axis] = error[axis];
    auto retained_distance = hardware::vector_length(error);
    const float stored_distance = float(retained_distance);
    if (retained_distance < X(0.f))
        retained_distance = -retained_distance;
    const float distance = float(retained_distance);
    PositionState state = at<PositionState>(car, 7940);
    if (position_only) {
        if (state == PositionState::Drone && 0.35f < time_gap)
            return;
        snapshots::copy_state(&candidate, &selected);
        if (!snapshots::camera_warp_requires_track_load(car, &candidate)) {
            body = at<void *>(car, 1768);
            for (unsigned field = 8; field <= 28; field += 4)
                at<Dword>(body, field) = 0;
            at<float>(body, 680) = candidate.position[0];
            at<Dword>(car, 3160) = 0;
            at<Dword>(car, 3164) = 0;
            at<Dword>(car, 3168) = 0;
            at<float>(body, 684) = candidate.position[1];
            at<float>(body, 688) = candidate.position[2];
            state = PositionState::Warp;
        } else
            state = PositionState::Catchup;
        at<float>(car, 7948) = stored_distance;
        at<PositionState>(car, 7940) = state;
        at<Dword>(at<void *>(car, 1768), 732) = 30;
        return;
    }
    const float speed = std::fabs(queue->speed);
    if (state == PositionState::Interpolate || state == PositionState::Null ||
        state == PositionState::Warp || (state == PositionState::Catchup && distance < 25.f))
        state = PositionState::Simulate;
    if (!(0.55f < time_gap)) {
        if (state == PositionState::Drone || state == PositionState::Guide) {
            if (!(time_gap < 0.35f) && !(speed < 0.44703f)) {
                if (time_gap < 3.f)
                    state = PositionState::Guide;
            } else
                state = PositionState::Simulate;
        }
    } else if (time_gap <= 5.f) {
        if (0.44703f < speed)
            state = PositionState::Guide;
    } else
        state = PositionState::Drone;
    if (future_snapshot) {
        if (state == PositionState::Guide)
            goto guide_small_error;
        if (state == PositionState::Simulate)
            goto simulated_small_error;
        if (state == PositionState::Catchup)
            goto recover_catchup;
        goto complete_transition;
    }
    if (state == PositionState::Simulate) {
        if (360.f < distance ||
            (at<void *>(car, 1768) && !at<Byte>(at<void *>(car, 1768), 728)))
            goto attempt_warp;
        {
            const auto traveled = hardware::vector_length(trajectory) + 6.f;
            if (traveled < X(distance))
                goto attempt_warp;
        }
        if (!(30.f < distance))
            goto simulated_small_error;
        state = PositionState::Catchup;
        goto recover_catchup;
    }
    if (state != PositionState::Guide) {
        if (state == PositionState::Catchup)
            goto recover_catchup;
        goto complete_transition;
    }
    if ((!(speed < 0.44703f) || !(6.f < distance)) && !(90.f < distance))
        goto guide_small_error;
attempt_warp:
    snapshots::copy_state(&candidate, &selected);
    if (snapshots::camera_warp_requires_track_load(car, &candidate)) {
        state = PositionState::Catchup;
        goto recover_catchup;
    }
    warp_to_snapshot(car, selected);
    state = PositionState::Warp;
    goto complete_transition;
guide_small_error:
    if (distance < 6.f && speed < 0.44703f)
        state = PositionState::Interpolate;
    goto complete_transition;
simulated_small_error:
    if (distance < 6.f && 0.01f < distance && speed < 0.0447030030190944671630859375f) {
        state = PositionState::Interpolate;
        goto complete_transition;
    }
recover_catchup : {
    const Dword now = network::transport_time();
    const float car_speed = std::fabs(at<float>(car, 3168));
    if (!(car_speed < 0.44703f) || distance < 6.f || std::isnan(distance))
        at<Dword>(car, 7944) = now;
    if (future_snapshot || static_cast<std::int32_t>(now - at<Dword>(car, 7944)) <= 500)
        goto select_ghost_period;
    float reset_speed;
    const Dword reset_speed_word = 1098536505;
    std::memcpy(&reset_speed, &reset_speed_word, 4);
    physics::request_car_reset(car, reset_speed);
    at<Dword>(car, 7944) = now;
    state = PositionState::Drone;
    goto notify_ghost_period;
}
complete_transition:
    at<Dword>(car, 7944) = network::transport_time();
select_ghost_period:
    if (state != PositionState::Catchup && state != PositionState::Guide &&
        state != PositionState::Drone) {
        if (selected.flags & 4)
            at<Dword>(at<void *>(car, 1768), 732) = 6;
        goto finish_state;
    }
notify_ghost_period:
    body = at<void *>(car, 1768);
    if (at<int>(body, 732) < 3) {
        at<Dword>(body, 732) = 30;
        const auto identifier =
            application::pending::lookup_player_identifier(system(104), at<Dword>(car, 1772));
        if (static_cast<std::int16_t>(identifier) >= 0)
            snapshots::send_ghost_period(system(112), identifier, 30);
    }
finish_state:
    if (at<int>(system(72), 152) == 1) {
        void *participant =
            application::pending::race_participant_owner_view(system(92), at<Dword>(car, 1772));
        if (participant && (network::race_pending(static_cast<Dword *>(participant)) & 255))
            at<Dword>(at<void *>(car, 1768), 732) = 90;
    }
    at<float>(car, 7948) = stored_distance;
    at<PositionState>(car, 7940) = state;
    if (state != PositionState::Catchup && state != PositionState::Warp && at<void *>(car, 1768))
        at<Byte>(at<void *>(car, 1768), 728) = 1;
}
} // namespace nfs::services::pending
