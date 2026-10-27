#include "camera_director_runtime.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "../../physics/bodies/shape_body_runtime.hpp"
namespace nfs::application::pending {
void pause_race_for_menu();
void resume_race_from_menu(std::uint8_t);
} // namespace nfs::application::pending
namespace nfs::game::world::camera {
using kernel::numeric::Extended;

void copy_actor_pose(void *state, const void *pose) {
    const Dword up[3] = {at<Dword>(pose, 4), at<Dword>(pose, 20), at<Dword>(pose, 36)};
    const float position[3] = {at<float>(pose, 48), at<float>(pose, 52), at<float>(pose, 56)};
    const float forward[3] = {at<float>(pose, 8), at<float>(pose, 24), at<float>(pose, 40)};
    const auto format = kernel::numeric::current_format();
    for (unsigned axis = 0; axis < 3; ++axis) {
        at<float>(state, 4 + axis * 4) = position[axis];
        at<float>(state, 16 + axis * 4) = float(Extended(position[axis], format) + forward[axis]);
        at<Dword>(state, 60 + axis * 4) = up[axis];
    }
}

void __fastcall advance_actor_transform(void *, void *, void *state) {
    copy_actor_pose(state, offset(subsystem(48), 16));
}

void __fastcall begin_actor_transform(void *, void *, void *state) {
    copy_actor_pose(state, offset(subsystem(48), 16));
}

void __fastcall advance_actor_clip(void *, void *, void *state) {
    at<float>(state, 52) = 0.1f;
}

int selected_layout() {
    if (network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 13) == 1)
        return 0;
    return network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 38) ? 2 : 1;
}

void __fastcall advance_follow_fov(void *behavior, void *, void *state) {
    if (!at<Byte>(behavior, 32))
        return;
    const auto format = kernel::numeric::current_format();
    Extended fov(at<float>(behavior, 4), format);
    const float speed = at<float>(at<void *>(behavior, 36), 3168);
    if (22.3515f < speed)
        fov = (Extended(speed, format) - 22.3515f) * 1.4f + fov;
    if (Extended(170.f, format) < fov)
        fov = Extended(170.f, format);
    at<float>(behavior, 8) = float(fov);
    // The FST to +8 leaves the unrounded value on the x87 stack.
    fov = (fov - at<float>(behavior, 12)) * 0.05f + at<float>(behavior, 12);
    at<float>(behavior, 12) = float(fov);
    at<float>(state, 28) = float(fov);
}

void __fastcall begin_follow_fov(void *behavior, void *, void *state) {
    if (!at<Dword>(state, 0))
        return;
    const int layout = selected_layout();
    if (layout >= 0 && layout <= 2) {
        const float fov = float(Extended(at<float>(behavior, 16 + layout * 4),
                                         kernel::numeric::current_format()));
        at<float>(behavior, 4) = fov;
        at<float>(behavior, 12) = fov;
        at<float>(behavior, 8) = fov;
    }
}

void __fastcall begin_follow_look(void *behavior, void *, void *) {
    at<Dword>(behavior, 132) = 0;
}

void __fastcall advance_point_look(void *behavior, void *, void *state) {
    if (const void *target = at<void *>(behavior, 4))
        for (unsigned axis = 0; axis < 3; ++axis)
            at<Dword>(state, 16 + axis * 4) = at<Dword>(target, axis * 4);
}

void __fastcall begin_mount_position(void *behavior, void *, void *) {
    runtime::physics::clear_ground_contact(offset(behavior, 36));
}

void __fastcall begin_matrix_controller(void *, void *) {
    application::pending::pause_race_for_menu();
    game::audio::mute_game_audio();
}

void __fastcall end_matrix_controller(void *, void *) {
    application::pending::resume_race_from_menu(1);
}

void __fastcall advance_matrix_controller(void *controller, void *) {
    runtime::pending::update_race_audio();
    for (unsigned field = 80; field <= 100; field += 4)
        if (void *behavior = at<void *>(controller, field))
            invoke(behavior, 1, offset(controller, 4));
}
} // namespace nfs::game::world::camera
