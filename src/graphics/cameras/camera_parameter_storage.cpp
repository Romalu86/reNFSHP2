#include "camera_director_runtime.hpp"
namespace nfs::game::world::camera {

void save_full_fov(void *object) {
    at<Dword>(object, 16) = at<Dword>(object, 4);
}

void save_vertical_fov(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 4);
}

void save_horizontal_fov(void *object) {
    at<Dword>(object, 24) = at<Dword>(object, 4);
}

void save_full_pitch(void *object) {
    at<Dword>(object, 12) = at<Dword>(object, 8);
}

void save_vertical_pitch(void *object) {
    at<Dword>(object, 16) = at<Dword>(object, 8);
}

void save_horizontal_pitch(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 8);
}

void save_full_up_tension(void *object) {
    at<Dword>(object, 28) = at<Dword>(object, 24);
}

void save_vertical_up_tension(void *object) {
    at<Dword>(object, 32) = at<Dword>(object, 24);
}

void save_horizontal_up_tension(void *object) {
    at<Dword>(object, 36) = at<Dword>(object, 24);
}

void save_full_mount_pitch(void *object) {
    at<Dword>(object, 24) = at<Dword>(object, 20);
}

void save_vertical_mount_pitch(void *object) {
    at<Dword>(object, 28) = at<Dword>(object, 20);
}

void save_horizontal_mount_pitch(void *object) {
    at<Dword>(object, 32) = at<Dword>(object, 20);
}

void save_full_mount_height(void *object) {
    at<Dword>(object, 24) = at<Dword>(object, 20);
}

void save_vertical_mount_height(void *object) {
    at<Dword>(object, 28) = at<Dword>(object, 20);
}

void save_horizontal_mount_height(void *object) {
    at<Dword>(object, 32) = at<Dword>(object, 20);
}

void save_full_interval(void *object) {
    at<Dword>(object, 36) = at<Dword>(object, 32);
}

void save_vertical_interval(void *object) {
    at<Dword>(object, 40) = at<Dword>(object, 32);
}

void save_horizontal_interval(void *object) {
    at<Dword>(object, 44) = at<Dword>(object, 32);
}

void save_full_follow_parameters(void *object) {
    at<Dword>(object, 140) = at<Dword>(object, 100);
    at<Dword>(object, 144) = at<Dword>(object, 104);
    at<Dword>(object, 148) = at<Dword>(object, 108);
    at<Dword>(object, 152) = at<Dword>(object, 112);
    at<Dword>(object, 156) = at<Dword>(object, 116);
    at<Dword>(object, 160) = at<Dword>(object, 120);
    at<Dword>(object, 164) = at<Dword>(object, 124);
    at<Dword>(object, 168) = at<Dword>(object, 128);
    at<Dword>(object, 172) = at<Dword>(object, 132);
    at<Dword>(object, 176) = at<Dword>(object, 136);
}

void save_vertical_follow_parameters(void *object) {
    at<Dword>(object, 180) = at<Dword>(object, 100);
    at<Dword>(object, 184) = at<Dword>(object, 104);
    at<Dword>(object, 188) = at<Dword>(object, 108);
    at<Dword>(object, 192) = at<Dword>(object, 112);
    at<Dword>(object, 196) = at<Dword>(object, 116);
    at<Dword>(object, 200) = at<Dword>(object, 120);
    at<Dword>(object, 204) = at<Dword>(object, 124);
    at<Dword>(object, 208) = at<Dword>(object, 128);
    at<Dword>(object, 212) = at<Dword>(object, 132);
    at<Dword>(object, 216) = at<Dword>(object, 136);
}

void save_horizontal_follow_parameters(void *object) {
    at<Dword>(object, 220) = at<Dword>(object, 100);
    at<Dword>(object, 224) = at<Dword>(object, 104);
    at<Dword>(object, 228) = at<Dword>(object, 108);
    at<Dword>(object, 232) = at<Dword>(object, 112);
    at<Dword>(object, 236) = at<Dword>(object, 116);
    at<Dword>(object, 240) = at<Dword>(object, 120);
    at<Dword>(object, 244) = at<Dword>(object, 124);
    at<Dword>(object, 248) = at<Dword>(object, 128);
    at<Dword>(object, 252) = at<Dword>(object, 132);
    at<Dword>(object, 256) = at<Dword>(object, 136);
}
} // namespace nfs::game::world::camera
