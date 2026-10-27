#include "camera_director_runtime.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "../../ai/driving/ai_route_planner.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::world::camera {
using kernel::numeric::Extended;
// Only AL is consumed by the caller.
Byte can_begin_jump_camera(void *director) {
    const Dword mode = at<Dword>(director, 312 + at<Dword>(director, 356) * 4);
    if (at<Byte>(director, 172) || (mode != 1 && mode != 0 && mode != 2 && mode != 3))
        return 0;
    void *car = focused_car();
    if (!car)
        return 0;
    void *entity = at<void *>(focused_car(), 7988);
    if (!entity || !network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 41) ||
        at<int>(subsystem(72), 176) != 1 || network::session_active())
        return 0;
    void *body = at<void *>(car, 1768);
    if (float(at<std::uint16_t>(body, 596)) <= at<float>(director, 184) ||
        at<Dword>(body, 732) || at<float>(car, 3168) < 22.3515f || at<Dword>(car, 448))
        return 0;
    if (!at<Byte>(entity, 409) &&
        route_heading_alignment(entity) < Extended(0.f, kernel::numeric::current_format()))
        return 0;
    return at<float>(body, 112) < 0.707f ? 0 : 1;
}

void update_jump_camera(void *director) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    if (!at<Byte>(director, 172) && !at<Byte>(director, 173)) {
        void *car = focused_car();
        if (!car)
            return;
        void *entity = at<void *>(focused_car(), 7988);
        if (!entity || !at<Dword>(director, 168) || !can_begin_jump_camera(director))
            return;
        float point[3], pose[21], width;
        Dword node;
        if (!runtime::ai::entities::pending::sample_path_target(
                offset(entity, 220), point, &width, &node, pose,
                float(X(at<float>(car, 3168)) * 0.5f), invoke<const float *>(entity, 0), 0))
            return;
        const auto lateral = route_lateral_position(entity);
        const float *edge = X(0.f) < lateral ? pose + 12 : pose + 15;
        for (unsigned axis = 0; axis < 3; ++axis)
            point[axis] = float(X(edge[axis]) + pose[axis]);
        point[2] = -point[2];
        if (at<Dword>(director, 168) == 1 || ++jump_variant > 2)
            jump_variant = 0;
        const auto select_preset = [&](unsigned preset) {
            at<Dword>(director, 256) = at<Dword>(director, preset + 12);
            at<Dword>(director, 252) = at<Dword>(director, preset + 8);
            at<Dword>(director, 244) = at<Dword>(director, preset);
            at<Dword>(director, 248) = at<Dword>(director, preset + 4);
        };
        switch (jump_variant) {
        case 0: select_preset(196); break;
        case 1: select_preset(212); break;
        case 2: select_preset(228); break;
        }
        void *body = at<void *>(car, 1768);
        alignas(4) Byte ground[84];
        std::memcpy(ground, offset(body, 508), sizeof(ground));
        if (!runtime::physics::surface_height(ground, point))
            return;
        if ((at<Byte>(at<void *>(body, 532), 59) ^ at<Byte>(at<void *>(ground, 24), 59)) &
            15)
            return;
        const auto raised_height = X(point[1]) + 2.f;
        const auto minimum_height = X(at<float>(body, 684)) + at<float>(director, 252);
        const float saved_minimum = float(minimum_height);
        point[1] = raised_height < minimum_height ? saved_minimum : float(raised_height);
        const Dword fov = at<Dword>(director, 248);
        push_mode(director, 4);
        void *state = invoke<void *>(director, 2);
        std::memcpy(offset(state, 4), point, 12);
        at<Dword>(state, 28) = at<Dword>(state, 32) = fov;
        void *target = runtime::physics::local_car(static_cast<Dword *>(subsystem(68)),
                                                   at<int>(director, 304));
        if (void *look = at<void *>(find_camera_mode(director, 4), 84))
            at<void *>(look, 4) = offset(at<void *>(target, 1768), 680);
        at<float>(director, 176) = frame_clock::step;
        const float speed = at<float>(director, 244);
        at<Dword>(director, 180) = 0;
        at<Byte>(director, 172) = at<Byte>(director, 173) = 1;
        frame_clock::step = speed > 0 ? speed : 0;
        return;
    }
    if (!at<Byte>(director, 172)) {
        if (at<std::int16_t>(at<void *>(focused_car(), 1768), 596))
            return;
        at<Byte>(director, 173) = 0;
        return;
    }
    void *car = focused_car();
    void *state = invoke<void *>(director, 2);
    if (state && car) {
        float point[3];
        std::memcpy(point, offset(state, 4), 12);
        const auto distance = runtime::retained_point_distance(
            reinterpret_cast<float *>(offset(at<void *>(car, 1768), 680)), point);
        if (X(1.f) < distance)
            at<float>(state, 28) =
                float(X(at<float>(state, 32)) - (X(0.5f) / distance) * at<float>(state, 32));
    }
    if (!frame_clock::paused)
        at<float>(director, 180) =
            float(X(0.01666666753590106964111328125f) / X(at<float>(director, 244)) + at<float>(director, 180));
    if (!(at<float>(director, 180) > at<float>(director, 192))) {
        if (!(at<float>(director, 180) > at<float>(director, 188)))
            return;
        if (at<std::int16_t>(at<void *>(focused_car(), 1768), 596))
            return;
        at<Byte>(director, 173) = 0;
    }
    if (invoke<void *>(director, 2) && at<Dword>(invoke<void *>(director, 2), 72) == 4) {
        at<Byte>(director, 172) = 0;
        pop_mode(director);
    }
    const float saved_step = at<float>(director, 176);
    frame_clock::step = saved_step > 0 ? saved_step : 0;
}
} // namespace nfs::game::world::camera
