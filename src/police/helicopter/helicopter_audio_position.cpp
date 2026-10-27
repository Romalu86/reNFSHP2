#include "../../math/vectors/retained_vector.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cmath>
namespace nfs::game::runtime::ai::lifecycle::pending {

void position_helicopter_sound(const float *position, const float *velocity) {
    using namespace race_audio;
    int listener = 0;
    int listener_count = race_audio::pending::race_audio_listener_count;
    while (listener < listener_count) {
        void *camera = game::tick_services[listener];
        auto *pose = invoke<Dword *>(camera, 2);
        float listener_position[3] = {at<float>(pose, 4), at<float>(pose, 8), at<float>(pose, 12)};
        float orientation[9];
        camera_orientation(pose, orientation);
        const float distance = float(retained_point_distance(listener_position, position));
        const int rounded = hardware::current_integer_f32(distance);
        const int bounded = rounded < 0 || rounded > 10000 ? 10000 : rounded;
        auto *target = race_audio::pending::listener_state + listener * 2760;
        at<volatile Dword>(target, 2808) = at<const Dword>(position, 0);
        at<volatile Dword>(target, 2812) = at<const Dword>(position, 4);
        at<volatile Dword>(target, 2816) = at<const Dword>(position, 8);
        at<volatile Dword>(target, 2820) = at<const Dword>(velocity, 0);
        const Dword velocity_y = at<const Dword>(velocity, 4);
        const Dword velocity_z = at<const Dword>(velocity, 8);
        at<volatile Dword>(target, 2832) = Dword(bounded);
        listener_count = race_audio::pending::race_audio_listener_count;
        ++listener;
        at<volatile Dword>(target, 2824) = velocity_y;
        at<volatile Dword>(target, 2828) = velocity_z;
    }
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
