#pragma once
#include "car_corridor_runtime.hpp"
#include "../population/ai_population_runtime.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "../../math/vectors/retained_vector.hpp"
namespace nfs::game::runtime::ai::cars {
void *construct_car_corridor(void *);
void copy_car_path(void *, const void *);
void *car_path(void *);
void *car_physical_object(void *);
Dword initialize_car_corridor(void *, void *, Dword, const void *, const float *, float, float);
Dword corridor_sample_edges(void *, int, float *, float *, float *, float *, float *);
void corridor_blend_axes(void *);
Dword corridor_measure_road(void *);
void corridor_score_target(void *, const float *, float);
Dword corridor_score_road_edges(void *, float);
Dword corridor_score_outer_road(void *, float);
Dword corridor_score_static_shapes(void *, float);
Dword corridor_point_on_road(void *, const float *);
Dword corridor_score_side(void *, int, float);
Dword corridor_select_target(void *, float *, float *);
Dword corridor_lane_position(void *, int, float *);
namespace corridor_detail {
using kernel::numeric::Extended;
inline auto format() {
    return kernel::numeric::current_format();
}
inline Extended number(float value) {
    return Extended(value, format());
}
inline Extended retained(long double value) {
    return Extended::from_double(double(value), format());
}
inline float *vector(void *object, unsigned offset) {
    return reinterpret_cast<float *>(bytes(object) + offset);
}
inline int project_lane(void *context, const float *point) {
    float copied[3], delta[3];
    physics::copy_triplet(copied, point);
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(copied[axis]) - at<float>(context, 644 + axis * 4));
    return corridor_lane_index(runtime::retained_dot_product(delta, vector(context, 680)));
}
inline int nearest_sample(void *context, const float *position) {
    void *path = bytes(context) + 708;
    const int last = lifecycle::path_length(path, 100);
    float best = 10000.f;
    int selected = 0;
    for (int sample = path_minimum_sample(path, -100); sample < last; ++sample) {
        float point[3], copied[3], delta[3];
        physics::copy_triplet(point, path_sample_position(path, sample));
        point[2] = -point[2];
        physics::copy_triplet(copied, position);
        values::difference_triplets(delta, copied, point);
        const auto distance = runtime::retained_vector_length(delta);
        if (distance < number(best)) {
            best = float(distance);
            selected = sample;
        }
    }
    return selected;
}
} // namespace corridor_detail
} // namespace nfs::game::runtime::ai::cars
