#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "race_frame_draw.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdlib>
#include <cstring>
#include <initializer_list>
namespace nfs::graphics {
void initialize_matrix_state();
}
namespace nfs::game::world::frame {
graphics::passes::ReflectionDistance reflection_distance;
// The constructor leaves distance untouched.
static graphics::passes::ReflectionDistance *construct_reflection_distance(
    graphics::passes::ReflectionDistance *settings) {
    settings->enabled = 1;
    return settings;
}

static void destroy_reflection_distance(graphics::passes::ReflectionDistance *settings) {
    settings->enabled = 0;
}

static void shutdown_reflection_distance() {
    destroy_reflection_distance(&reflection_distance);
}

static void initialize_reflection_distance() {
    construct_reflection_distance(&reflection_distance);
    std::atexit(shutdown_reflection_distance);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_initialize_reflection_distance, ".CRT$XCU_NFS_006901E8",
                       initialize_reflection_distance);
NFS_KERNEL_INITIALIZER(startup_initialize_positional_light_sources, ".CRT$XCU_NFS_006901EC",
                       graphics::initialize_positional_light_sources);
NFS_KERNEL_INITIALIZER(startup_initialize_matrix_state, ".CRT$XCU_NFS_006901F0",
                       graphics::initialize_matrix_state);
}

void set_camera_field_of_view(float value) {
    graphics::camera_field_of_view = value;
}

void update_reflection_directions(void *environment, void *director) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return kernel::numeric::Extended(value, format); };
    float forward[3], side[3], up[3];
    if (dynamic_reflection_enabled()) {
        void *car = runtime::physics::local_car(static_cast<Dword *>(subsystem(68)), 0);
        float rotation[9];
        std::memcpy(rotation, offset(at<void *>(car, 1768), 692), sizeof rotation);
        for (unsigned axis = 0; axis < 3; ++axis) {
            forward[axis] = -rotation[6 + axis];
            side[axis] = -rotation[axis];
            up[axis] = rotation[3 + axis];
        }
    } else {
        const auto *pose = invoke<const float *>(director, 2);
        for (unsigned axis = 0; axis < 3; ++axis)
            forward[axis] = float(E(pose[1 + axis]) - at<float>(environment, 164 + axis * 4));
        const float vertical[3] = {0, -1, 0};
        runtime::cross_vectors_entry(forward, vertical, side);
        runtime::cross_vectors_entry(forward, side, up);
    }
    float backward[3], down[3], right[3];
    for (unsigned axis = 0; axis < 3; ++axis) {
        backward[axis] = -forward[axis];
        down[axis] = -up[axis];
        right[axis] = -side[axis];
    }
    float normalized_up[3], top_up[3], bottom_up[3];
    runtime::normalize_vector(up, normalized_up);
    runtime::cross_vectors_entry(down, side, top_up);
    runtime::cross_vectors_entry(up, side, bottom_up);
    auto translate = [&](float *vector) {
        for (unsigned axis = 0; axis < 3; ++axis)
            vector[axis] = float(E(vector[axis]) + at<float>(environment, 164 + axis * 4));
    };
    translate(forward);
    translate(backward);
    translate(right);
    translate(side);
    translate(up);
    translate(down);
    std::memcpy(offset(environment, 176), up, 12);
    std::memcpy(offset(environment, 200), forward, 12);
    auto *rules = static_cast<Dword *>(subsystem(72));
    bool flip = network::packets::rule_value(rules, 6) == 2;
    if (!flip)
        flip = network::packets::rule_value(rules, 6) == 3;
    if (flip) {
        std::memcpy(offset(environment, 212), right, 12);
        std::memcpy(offset(environment, 188), side, 12);
    } else {
        std::memcpy(offset(environment, 188), right, 12);
        std::memcpy(offset(environment, 212), side, 12);
    }
    std::memcpy(offset(environment, 224), backward, 12);
    std::memcpy(offset(environment, 236), down, 12);
    std::memcpy(offset(environment, 248), bottom_up, 12);
    for (unsigned destination : {272u, 260u, 284u, 296u})
        std::memcpy(offset(environment, destination), normalized_up, 12);
    std::memcpy(offset(environment, 308), top_up, 12);
}
} // namespace nfs::game::world::frame
