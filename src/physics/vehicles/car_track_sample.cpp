#include "car_physics_frame.hpp"
#include "car_drive_frame_x87.hpp"
namespace nfs::game::runtime::physics::car_frame {

int nearest_track_sample(void *track, const float *position, int sample) {
    void *layout = at<void *>(track, 28);
    if (!layout)
        return 0;
    const int count = at<int>(layout, 20);
    if (!count)
        return 0;
    int previous = int(Dword(sample) - 1), next = int(Dword(sample) + 1);
    if (previous < 0)
        previous = int(Dword(count) - 1);
    if (count <= next)
        next = 0;
    const float *points = at<const float *>(layout, 32);
    const auto distance = [&](int index) {
        const auto x = drive::retained(position[0]) - points[Dword(index) * 3];
        const auto z = drive::retained(position[2]) - points[Dword(index) * 3 + 2];
        return z * z + x * x;
    };
    const float before = float(distance(previous)), here = float(distance(sample)),
                after = float(distance(next));
    if (!(here > 15000)) {
        if (here < before && here < after)
            return sample;
        return after < here && after < before ? next : previous;
    }
    int closest = 0;
    float minimum = float(distance(0));
    for (int index = 1; index < count; ++index) {
        const auto candidate = distance(index);
        if (candidate < drive::retained(minimum)) {
            closest = index;
            minimum = float(candidate);
        }
    }
    return closest;
}

void update_car_track_sample(void *car) {
    void *body = at<void *>(car, 1768);
    float position[3] = {at<float>(body, 680), at<float>(body, 684), -at<float>(body, 688)};
    at<int>(car, 7984) = nearest_track_sample(track_collision, position, at<int>(car, 7984));
}
} // namespace nfs::game::runtime::physics::car_frame
