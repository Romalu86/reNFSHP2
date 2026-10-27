#include "car_corridor_x87_private.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace corridor_detail;

Dword corridor_measure_road(void *context) {
    const float range = float(number(at<float>(context, 692)) * at<float>(context, 696));
    const int end = lifecycle::path_length(bytes(context) + 708, 20);
    float left[3] = {}, right[3] = {}, lateral[3] = {}, forward[3] = {}, center[3] = {};
    const bool nonnegative = !(at<float>(context, 804) < 0.f);
    const int first = nonnegative ? 2 : 1;
    if (!corridor_sample_edges(context, nonnegative ? 1 : 0, left, right, lateral, forward, center))
        return 0;
    const float heading = float(runtime::turn_direction_retained(forward[0], forward[2]));
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(center[axis]) - at<float>(context, 644 + axis * 4));
    float distance = float(hardware::dot_product(forward, delta));
    int minimum = project_lane(context, left), maximum = project_lane(context, right);
    if (minimum > maximum)
        std::swap(minimum, maximum);
    if (minimum == maximum)
        return 0;
    at<int>(context, 800) = first;
    while (at<int>(context, 800) < end) {
        float next_left[3] = {}, next_right[3] = {}, next_lateral[3] = {}, next_forward[3] = {},
              next[3] = {};
        if (corridor_sample_edges(context, at<int>(context, 800), next_left, next_right,
                                  next_lateral, next_forward, next)) {
            for (unsigned axis = 0; axis < 3; ++axis)
                delta[axis] = float(number(next[axis]) - center[axis]);
            const auto length = hardware::vector_length(delta);
            if (length != number(0)) {
                distance = float(length + distance);
                const auto turn =
                    runtime::turn_direction_retained(next_forward[0], next_forward[2]) - heading;
                if (range <= distance)
                    break;
                if (number(.125f) < turn || turn < number(-.125f))
                    break;
                physics::copy_triplet(center, next);
            }
        }
        at<Dword>(context, 800) += 1;
    }
    if (distance <= 0.f)
        return 0;
    for (unsigned lane = 0; lane < 40; ++lane) {
        at<float>(context, 160 + lane * 4) = distance;
        auto time = number(distance) / number(at<float>(context, 692));
        if (number(at<float>(context, 696)) < time)
            time = number(at<float>(context, 696));
        at<float>(context, 320 + lane * 4) = float(time);
    }
    return 1;
}

Dword initialize_car_corridor(void *context, void *car, Dword horizon_bits, const void *path,
                             const float *target, float speed, float penalty) {
    alignas(4) Byte argument_path[88];
    copy_car_path(argument_path, path);
    struct PathArgument {
        void *path;
        ~PathArgument() {
            lifecycle::pending::destroy_population_path(path);
        }
    } argument_owner{argument_path};
    const float horizon = float_word(horizon_bits);
    if (!car || horizon <= 0.f || !entities::path_has_section(argument_path)) {
        return 0;
    }
    for (unsigned base : {0u, 160u, 480u, 320u})
        for (unsigned lane = 0; lane < 40; ++lane)
            at<Dword>(context, base + lane * 4) = 0;
    for (unsigned offset = 0; offset <= 36; offset += 4)
        at<Dword>(context, 708 + offset) = at<Dword>(argument_path, offset);
    lifecycle::copy_path_reference(bytes(context) + 748, argument_path + 40);
    float pose[21];
    lifecycle::sample_path(bytes(context) + 708, 0, pose);
    const auto *position = invoke<const float *>(at<void *>(car, 7988), 0);
    float delta[3] = {float(number(pose[0]) - position[0]), float(number(pose[1]) - position[1]),
                      float(number(pose[2]) - position[2])};
    auto offset = hardware::dot_product(delta, pose + 3) * -1.f;
    at<float>(context, 804) = float(offset);
    if (at<int>(context, 728) < 0)
        at<float>(context, 804) = float(offset * -1.f);
    void *body = at<void *>(car, 1768);
    at<void *>(context, 640) = car;
    physics::copy_triplet(vector(context, 644), vector(body, 680));
    corridor_blend_axes(context);
    body = at<void *>(car, 1768);
    physics::copy_triplet(vector(context, 656), vector(body, 8));
    at<float>(context, 692) =
        float(hardware::dot_product(vector(context, 656), vector(context, 668)));
    at<float>(context, 696) = horizon;
    at<int>(context, 796) = project_lane(context, vector(context, 644));
    at<float>(context, 808) = speed;
    if (!corridor_measure_road(context)) {
        return 0;
    }
    float desired[3] = {target[0], target[1], -target[2]};
    corridor_score_target(context, desired, penalty);
    return 1;
}

Dword corridor_score_road_edges(void *context, float penalty) {
    float left[3] = {}, right[3] = {}, lateral[3] = {}, forward[3] = {}, center[3] = {};
    if (!corridor_sample_edges(context, at<float>(context, 804) > 0.f, left, right, lateral,
                               forward, center))
        return 0;
    int minimum = project_lane(context, left), maximum = project_lane(context, right);
    if (minimum == maximum)
        return 0;
    if (minimum > maximum)
        std::swap(minimum, maximum);
    const auto add = [&](int lane, long double increment) {
        auto &cost = at<float>(context, lane * 4);
        cost = float(increment + cost);
    };
    if (minimum < 37)
        add(minimum + 3, number(penalty) * .25f);
    if (minimum < 38)
        add(minimum + 2, number(penalty) * .5f);
    if (minimum < 39)
        add(minimum + 1, number(penalty));
    if (maximum != 0)
        add(maximum - 1, number(penalty));
    if (maximum > 1)
        add(maximum - 2, number(penalty) * .5f);
    if (maximum > 2)
        add(maximum - 3, number(penalty) * .25f);
    return 1;
}

void add_corridor_lane_cost(void *context, int lane, float penalty) {
    const Dword offset = Dword(lane) * 4;
    at<float>(context, offset) = float(number(penalty) + at<float>(context, offset));
}

Dword corridor_score_outer_road(void *context, float penalty) {
    const auto doubled_speed =
        number(at<float>(context, 692)) + at<float>(context, 692);
    if (doubled_speed <= number(0))
        return 0;
    float left[3] = {}, right[3] = {}, lateral[3] = {}, forward[3] = {}, center[3] = {};
    if (!corridor_sample_edges(context, !(at<float>(context, 804) < 0.f), left, right, lateral,
                               forward, center))
        return 0;
    int minimum = project_lane(context, left), maximum = project_lane(context, right);
    if (minimum == maximum)
        return 0;
    if (minimum > maximum)
        std::swap(minimum, maximum);
    auto score = number(penalty);
    for (int lane = minimum; lane >= 0; --lane) {
        at<float>(context, lane * 4) = float(score + at<float>(context, lane * 4));
        score = score * 1.05f;
    }
    score = number(penalty);
    for (int lane = maximum; lane < 40; ++lane) {
        at<float>(context, lane * 4) = float(score + at<float>(context, lane * 4));
        score = score * 1.05f;
    }
    corridor_score_static_shapes(context, penalty);
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
