#include "camera_controller_runtime.hpp"
#include <cstring>
namespace nfs::game::world::camera {

void active_camera_position(void *director, float *output) {
    void *mode =
        find_camera_mode(director, at<Dword>(director, 312 + at<Dword>(director, 356) * 4));
    std::memcpy(output, bytes(mode) + 8, 12);
}

void state_orientation(void *state, float *output) {
    float forward[3], side[3], up[3];
    std::memcpy(up, bytes(state) + 60, 12);
    const auto format = kernel::numeric::current_format();
    for (unsigned axis = 0; axis < 3; ++axis)
        forward[axis] = float(kernel::numeric::Extended(at<float>(state, 16 + axis * 4), format) -
                              at<float>(state, 4 + axis * 4));
    runtime::normalize_vector(forward, forward);
    runtime::cross_vectors_entry(up, forward, side);
    runtime::normalize_vector(side, side);
    runtime::cross_vectors_entry(forward, side, up);
    std::memcpy(output, side, 12);
    std::memcpy(output + 3, up, 12);
    std::memcpy(output + 6, forward, 12);
}
} // namespace nfs::game::world::camera
