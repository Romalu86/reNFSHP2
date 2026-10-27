#pragma once
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::runtime::physics::boxes {
extern float relative_rotation[9], relative_first[3], relative_second[3], scaled_first[9],
    scaled_second[9], probe_reversed[3];
int second_face(void *, void *, float, float, float);
int first_face(void *, void *, float, float, float);
int first_sample(void *, void *, float, float, float);
void select_normal(void *, void *, Dword);
inline bool inside(void *body) {
    return probe_reversed[0] <= at<float>(body, 128) &&
           -at<float>(body, 128) <= probe_reversed[0] &&
           probe_reversed[2] <= at<float>(body, 120) &&
           -at<float>(body, 120) <= probe_reversed[2] &&
           probe_reversed[1] <= at<float>(body, 124) &&
           -at<float>(body, 124) <= probe_reversed[1];
}
} // namespace nfs::game::runtime::physics::boxes
