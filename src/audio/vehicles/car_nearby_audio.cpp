#include "../race/race_audio_distance.hpp"
#include "../../vehicles/frame/car_frame_runtime.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace physics::car_frame;

void submit_nearby_audio_car(void *car, Dword kind) {
    using namespace race_audio::pending;
    for (int listener = 0; listener < race_audio_listener_count; ++listener) {
        auto *camera = invoke<Dword *>(game::tick_services[listener], 2);
        float position[3], orientation[23];
        std::memcpy(position, bytes(camera) + 4, 12);
        race_audio::camera_orientation(camera, orientation);
        const int rounded = rounded_point_distance(
            position, vector_at(at<void *>(car, 1768), 680));
        if (rounded < 0 || rounded > 10000 || rounded >= 75)
            continue;
        Byte *state = listener_state + Dword(listener) * 2760;
        int &count = at<int>(state, 220);
        if (count < 10) {
            at<void *>(state, 224 + Dword(count) * 16) = car;
            at<Dword>(state, 236 + Dword(count) * 16) = kind;
            at<int>(state, 232 + Dword(count) * 16) = rounded;
            ++at<Dword>(state, 220);
        } else {
            int maximum = 0, slot = -1;
            for (int index = 0; index < 10; ++index) {
                const int previous = at<int>(state, 232 + Dword(index) * 16);
                if (maximum < previous) {
                    slot = index;
                    maximum = previous;
                }
            }
            if (rounded < maximum) {
                at<void *>(state, 224 + Dword(slot) * 16) = car;
                at<Dword>(state, 236 + Dword(slot) * 16) = kind;
                at<int>(state, 232 + Dword(slot) * 16) = rounded;
            }
        }
    }
}
} // namespace nfs::game::runtime::ai::cars
