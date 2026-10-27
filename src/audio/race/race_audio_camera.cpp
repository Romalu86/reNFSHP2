#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/core/frame/game_step.hpp"
namespace nfs::game::runtime::race_audio {
// Hidden return-buffer ABI is represented explicitly.
void camera_position(Dword *camera, Dword *output) {
    Dword *position;
    if (at<Dword>(camera, 72) == 3 && at<void *>(tick_services[0], 300))
        position = reinterpret_cast<Dword *>(
            bytes(at<void *>(at<void *>(tick_services[0], 300), 1768)) + 680);
    else
        position = camera + 1;
    const Dword x = position[0], z = position[2];
    output[0] = x;
    output[1] = position[1];
    output[2] = z;
}

void camera_orientation(Dword *camera, float *output) {
    float forward[3], right[3], vertical[3], up[3];
    for (unsigned i = 0; i < 3; ++i) {
        forward[i] = at<float>(camera, 16 + i * 4) - at<float>(camera, 4 + i * 4);
        up[i] = at<float>(camera, 60 + i * 4);
    }
    normalize_vector(forward, forward);
    cross_vectors_entry(up, forward, right);
    normalize_vector(right, right);
    cross_vectors_entry(forward, right, vertical);
    for (unsigned i = 0; i < 3; ++i) {
        output[i] = right[i];
        output[3 + i] = vertical[i];
        output[6 + i] = forward[i];
    }
}
} // namespace nfs::game::runtime::race_audio
