#include "camera_director_runtime.hpp"
#include "nfs/gui/engine/gui_engine.hpp"
namespace nfs::game::world::camera {

void destroy_actor_transform(void *behavior) {
    at<const Dword *>(behavior, 0) = actor_transform_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_actor_transform(void *behavior, void *, Dword flags) {
    destroy_actor_transform(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_actor_clip(void *behavior) {
    at<const Dword *>(behavior, 0) = actor_clip_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_actor_clip(void *behavior, void *, Dword flags) {
    destroy_actor_clip(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_follow_fov(void *behavior) {
    at<const Dword *>(behavior, 0) = follow_fov_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_follow_fov(void *behavior, void *, Dword flags) {
    destroy_follow_fov(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_follow_look(void *behavior) {
    at<const Dword *>(behavior, 0) = follow_look_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_follow_look(void *behavior, void *, Dword flags) {
    destroy_follow_look(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_mount_look(void *behavior) {
    at<const Dword *>(behavior, 0) = mount_look_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_mount_look(void *behavior, void *, Dword flags) {
    destroy_mount_look(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_point_look(void *behavior) {
    at<const Dword *>(behavior, 0) = point_look_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_point_look(void *behavior, void *, Dword flags) {
    destroy_point_look(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_follow_position(void *behavior) {
    at<const Dword *>(behavior, 0) = follow_position_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_follow_position(void *behavior, void *, Dword flags) {
    destroy_follow_position(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_matrix_position(void *behavior) {
    at<const Dword *>(behavior, 0) = matrix_position_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_matrix_position(void *behavior, void *, Dword flags) {
    destroy_matrix_position(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_mount_position(void *behavior) {
    at<const Dword *>(behavior, 0) = mount_position_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_mount_position(void *behavior, void *, Dword flags) {
    destroy_mount_position(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_pause_position(void *behavior) {
    at<const Dword *>(behavior, 0) = pause_position_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_pause_position(void *behavior, void *, Dword flags) {
    destroy_pause_position(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}

void destroy_follow_up(void *behavior) {
    at<const Dword *>(behavior, 0) = follow_up_methods;
    at<const Dword *>(behavior, 0) = position_base_methods;
}

void *__fastcall delete_follow_up(void *behavior, void *, Dword flags) {
    destroy_follow_up(behavior);
    if (flags & 1)
        gui::application_release(behavior);
    return behavior;
}
} // namespace nfs::game::world::camera
