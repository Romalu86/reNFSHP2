#include "car_corridor_x87_private.hpp"
#include "nfs/world/lifecycle/world_dependency.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace corridor_detail;

Dword corridor_score_static_shapes(void *context, float penalty) {
    if (at<float>(context, 692) <= 0.f)
        return 0;
    const float range = float(number(at<float>(context, 696)) * at<float>(context, 692));
    int cell_x, cell_z;
    physics::cell_coordinates(vector(context, 644), &cell_x, &cell_z);
    const auto last_x = std::int32_t(Dword(cell_x) + 4);
    const auto first_z = std::int32_t(Dword(cell_z) - 4);
    const auto first_x = std::int32_t(Dword(cell_x) - 4);
    const auto last_z = std::int32_t(Dword(cell_z) + 4);
    for (auto x = first_x; x <= last_x; x = std::int32_t(Dword(x) + 1)) {
        for (auto z = first_z; z <= last_z; z = std::int32_t(Dword(z) + 1)) {
            void *cell = physics::cell(x, z);
            if (!cell)
                continue;
            for (int item = 0; item < at<int>(cell, 0); ++item) {
                const auto index = at<const std::uint16_t *>(cell, 4)[item];
                const auto *shape = static_cast<const float *>(
                    world::dependency::static_track_shape(system(24), index));
                if (!shape)
                    continue;
                float vertices[4][3] = {};
                for (unsigned corner = 0; corner < 4; ++corner) {
                    vertices[corner][0] = shape[corner * 2];
                    vertices[corner][2] = shape[corner * 2 + 1];
                }
                const float partial_x =
                    float(number(vertices[1][0]) + vertices[0][0] + vertices[2][0]);
                const float sum_z = float(number(vertices[1][2]) + vertices[0][2] + vertices[2][2] +
                                          vertices[3][2]);
                float center[3] = {float((number(partial_x) + vertices[3][0]) * .25f),
                                   at<float>(context, 648), float(number(sum_z) * .25f)};
                float delta[3];
                for (unsigned axis = 0; axis < 3; ++axis)
                    delta[axis] =
                        float(number(center[axis]) - at<float>(context, 644 + axis * 4));
                const auto projection =
                    hardware::dot_product(delta, vector(context, 668));
                const float distance = float(projection);
                if (projection < number(0) || range < distance ||
                    !corridor_point_on_road(context, center))
                    continue;
                int minimum = project_lane(context, center), maximum = minimum;
                for (const auto &vertex : vertices) {
                    const int lane = project_lane(context, vertex);
                    if (lane < minimum)
                        minimum = lane;
                    if (maximum < lane)
                        maximum = lane;
                }
                if (minimum < 0 || maximum < 0)
                    continue;
                if (minimum > maximum)
                    std::swap(minimum, maximum);
                minimum = audio::vehicle::clamp_value(minimum - 3, 0, 39);
                maximum = audio::vehicle::clamp_value(maximum + 3, 0, 39);
                const int target = at<int>(context, 704);
                if (minimum <= target && target <= maximum)
                    continue;
                const float cost = float((number(1) - number(distance) / number(range)) * penalty);
                for (int lane = minimum; lane <= maximum; ++lane) {
                    if (distance < at<float>(context, 160 + lane * 4)) {
                        auto value = number(cost);
                        if (number(0) < value * at<float>(context, lane * 4))
                            value = value + at<float>(context, lane * 4);
                        at<float>(context, lane * 4) = float(value);
                        at<float>(context, 160 + lane * 4) = distance;
                    }
                }
            }
        }
    }
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
