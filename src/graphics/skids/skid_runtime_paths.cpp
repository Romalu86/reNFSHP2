#include "../vehicles/car_wheel_effects_runtime.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "../../math/vectors/retained_vector.hpp"
#include <cmath>
#include <cstring>
namespace nfs::game::runtime::skids::lifecycle {
using kernel::numeric::Extended;

Byte skid_finished(void *path) {
    return at<Byte>(path, 72);
}

void release_skid_segment(void *segment) {
    release_skid(segment);
}

void release_skid_path(void *path) {
    if (!path)
        return;
    void *segment = at<void *>(path, 0);
    while (segment) {
        void *previous = at<void *>(segment, 56);
        release_skid_segment(segment);
        segment = previous;
    }
    release_skid(path);
}

void remove_skid_path(void *owner, void *path) {
    void *previous = at<void *>(path, 76), *next = at<void *>(path, 80);
    if (previous)
        at<void *>(previous, 80) = next;
    if (next)
        at<void *>(next, 76) = previous;
    if (at<void *>(owner, 4) == path)
        at<void *>(owner, 4) = previous;
    if (at<void *>(owner, 8) == path)
        at<void *>(owner, 8) = next;
    release_skid_path(path);
}
namespace {
void reclaim_finished_path(void *owner) {
    for (void *path = at<void *>(owner, 8); path; path = at<void *>(path, 80))
        if (skid_finished(path)) {
            remove_skid_path(owner, path);
            break;
        }
}
} // namespace

void *allocate_skid_path(void *owner) {
    void *path = allocate_skid();
    if (!path) {
        reclaim_finished_path(owner);
        path = allocate_skid();
        if (!path)
            return nullptr;
    }
    const auto format = kernel::numeric::current_format();
    void *tail = at<void *>(owner, 4);
    at<void *>(path, 76) = tail;
    at<Dword>(path, 80) = 0;
    at<float>(path, 32) =
        float(Extended::integer(int(graphics::unresolved::camera_state), format) *
              0.01666666753590106964111328125f);
    if (tail)
        at<void *>(tail, 80) = path;
    at<void *>(owner, 4) = path;
    if (!at<void *>(owner, 8))
        at<void *>(owner, 8) = path;
    return path;
}

Byte append_skid_segment(void *path, const float *point, float width, Dword material,
                         float strength) {
    void *segment = allocate_skid();
    if (!segment)
        return 0;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(path, 48 + axis * 4) = float(E(at<float>(path, 36 + axis * 4)) - point[axis]);
    at<Dword>(path, 52) = 0;
    physics::pending::normalize_collision_entry(&at<float>(path, 48));
    const float up[3] = {0, 1, 0};
    float perpendicular[3];
    cross_vectors_entry(&at<float>(path, 48), up, perpendicular);
    physics::pending::normalize_collision_entry(perpendicular);
    const float half_width = float(E(width) * .5f);
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(path, 60 + axis * 4) = float(E(perpendicular[axis]) * half_width);
    for (unsigned axis = 0; axis < 3; ++axis) {
        const auto origin = E(at<float>(path, 36 + axis * 4));
        const auto edge = E(at<float>(path, 60 + axis * 4));
        at<float>(segment, axis * 4) = float(edge + origin);
        at<float>(segment, 12 + axis * 4) = float(origin - edge);
    }
    at<float>(segment, 48) = strength;
    at<Dword>(segment, 52) = material;
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(segment, 24 + axis * 4) =
            float(E(point[axis]) - at<float>(path, 60 + axis * 4));
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(segment, 36 + axis * 4) =
            float(E(at<float>(path, 60 + axis * 4)) + point[axis]);
    std::memcpy(bytes(path) + 36, point, 12);
    void *previous = at<void *>(path, 0);
    if (previous)
        at<void *>(previous, 60) = segment;
    at<void *>(path, 0) = segment;
    at<void *>(segment, 56) = previous;
    return 1;
}
// Only AL is consumed by both callers.
Byte update_skid_path(void *path, const float *point, float width, Dword material, float strength) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    if (!at<void *>(path, 0)) {
        if (at<float>(path, 36) == 0 && at<float>(path, 40) == 0 &&
            at<float>(path, 44) == 0) {
            std::memcpy(bytes(path) + 36, point, 12);
            return 1;
        }
        const auto x = E(at<float>(path, 36)) - point[0], z = E(at<float>(path, 44)) - point[2];
        if (z * z + x * x < E(width) * width)
            return 1;
        return append_skid_segment(path, point, width, material, strength);
    }
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(E(at<float>(path, 36 + axis * 4)) - point[axis]);
    const auto retained_z = E(at<float>(path, 44)) - point[2];
    if (retained_z * delta[2] + E(delta[0]) * delta[0] < E(width) * width)
        return 1;
    float direction[3] = {delta[0], 0, delta[2]};
    physics::pending::normalize_collision_entry(direction);
    const float alignment = float(runtime::retained_dot_product(&at<float>(path, 48), direction));
    const float threshold = float(std::cos(double(0.08726499974727630615234375f)));
    if (threshold < alignment) {
        float near_edge[3], far_edge[3];
        for (unsigned axis = 0; axis < 3; ++axis)
            near_edge[axis] = float(E(point[axis]) - at<float>(path, 60 + axis * 4));
        for (unsigned axis = 0; axis < 3; ++axis)
            far_edge[axis] = float(E(at<float>(path, 60 + axis * 4)) + point[axis]);
        void *segment = at<void *>(path, 0);
        std::memcpy(bytes(segment) + 24, near_edge, 12);
        std::memcpy(bytes(segment) + 36, far_edge, 12);
        std::memcpy(bytes(path) + 36, point, 12);
        return 1;
    }
    return append_skid_segment(path, point, width, material, strength);
}
// Three velocity stack words are unused; RET20h retains them.
void update_skid(void *owner, void *path, const float *point, float, float, float, float width,
                 Dword material, float strength) {
    if (!update_skid_path(path, point, width, material, strength)) {
        reclaim_finished_path(owner);
        update_skid_path(path, point, width, material, strength);
    }
}
} // namespace nfs::game::runtime::skids::lifecycle
