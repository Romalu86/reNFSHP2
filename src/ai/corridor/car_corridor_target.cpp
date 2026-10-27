#include "car_corridor_x87_private.hpp"
namespace nfs::game::runtime::ai::cars {
using namespace corridor_detail;

void corridor_score_target(void *context, const float *requested, float penalty) {
    float target[3];
    physics::copy_triplet(target, requested);
    const int sample = nearest_sample(context, target);
    if (at<int>(context, 800) < sample) {
        float left[3] = {}, right[3] = {}, lateral[3] = {}, forward[3] = {}, distant[3] = {},
              current[3] = {};
        corridor_sample_edges(context, sample, left, right, lateral, forward, distant);
        corridor_sample_edges(context, at<int>(context, 800), left, right, lateral, forward,
                              current);
        const auto dx = number(target[0]) - distant[0];
        const auto dy = number(target[1]) - distant[1];
        const float dz = float(number(target[2]) - distant[2]);
        target[0] = float(dx + current[0]);
        target[1] = float(number(current[1]) + dy);
        target[2] = float(number(current[2]) + dz);
    }
    const int selected = project_lane(context, target);
    at<int>(context, 704) = selected;
    auto weight = number(.9f);
    for (int lane = selected - 1; lane >= 0; --lane) {
        auto &cost = at<float>(context, lane * 4);
        cost = float(number(penalty) * weight + cost);
        weight = weight * .9f;
    }
    auto &center_cost = at<float>(context, selected * 4);
    center_cost = float(number(penalty) + center_cost);
    weight = number(.9f);
    for (int lane = selected + 1; lane < 40; ++lane) {
        auto &cost = at<float>(context, lane * 4);
        cost = float(number(penalty) * weight + cost);
        weight = weight * .9f;
    }
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(target[axis]) - at<float>(context, 644 + axis * 4));
    auto distance = hardware::dot_product(delta, vector(context, 668));
    if (!(number(0) < distance))
        distance = number(at<float>(context, 696)) * at<float>(context, 692);
    at<float>(context, 700) = float(distance);
}

Dword corridor_lane_position(void *context, int lane, float *output) {
    const int center = at<int>(context, 796);
    const float speed = at<float>(context, 692);
    auto distance = number(at<float>(context, 696)) * speed;
    const float center_time_distance =
        float(number(at<float>(context, 320 + center * 4)) * speed);
    if (!(distance < number(center_time_distance)))
        distance = number(center_time_distance);
    const float lane_time_distance = float(number(at<float>(context, 320 + lane * 4)) * speed);
    if (!(distance < number(lane_time_distance)))
        distance = number(lane_time_distance);
    const float center_distance = at<float>(context, 160 + center * 4);
    if (!(distance < number(center_distance)))
        distance = number(center_distance);
    const float lane_distance = at<float>(context, 160 + lane * 4);
    if (!(distance < number(lane_distance)))
        distance = number(lane_distance);
    const Dword target_distance_bits = at<Dword>(context, 700);
    const float displacement = float(std::int32_t(Dword(lane) - 20));
    const float target_distance = float_word(target_distance_bits);
    if (!(distance < number(target_distance)))
        distance = number(target_distance);
    if (!(number(22) < distance))
        distance = number(22);
    const float forward_x = float(distance * at<float>(context, 668));
    const float forward_y = float(distance * at<float>(context, 672));
    const auto forward_z = distance * at<float>(context, 676);
    const auto side_x = number(displacement) * at<float>(context, 680);
    const auto side_y = number(displacement) * at<float>(context, 684);
    const float side_z = float(number(displacement) * at<float>(context, 688));
    const float combined_x = float(side_x + forward_x);
    const auto combined_y = side_y + forward_y;
    const float combined_z = float(number(side_z) + forward_z);
    const float x = float(number(combined_x) + at<float>(context, 644));
    const float y = float(combined_y + at<float>(context, 648));
    const auto z = number(combined_z) + at<float>(context, 652);
    output[0] = x;
    output[1] = y;
    output[2] = float(z);
    return 1;
}

Dword corridor_select_target(void *context, float *position, float *speed) {
    int right = at<int>(context, 796), left = right;
    float score = at<float>(context, right * 4);
    while (left > 0 && score <= at<float>(context, (left - 1) * 4)) {
        --left;
        score = at<float>(context, left * 4);
    }
    score = at<float>(context, right * 4);
    while (right + 1 < 40 && score <= at<float>(context, (right + 1) * 4)) {
        ++right;
        score = at<float>(context, right * 4);
    }
    if (!(at<float>(context, left * 4) < at<float>(context, right * 4)))
        right = left;
    float target[3] = {};
    if (!corridor_lane_position(context, right, target))
        return 0;
    position[0] = target[0];
    position[1] = target[1];
    position[2] = -target[2];
    *speed = at<float>(context, 808);
    return 1;
}

Dword corridor_score_side(void *context, int right_side, float penalty) {
    const float *sample = path_sample_position(bytes(context) + 708, 0);
    float position[3] = {sample[0], sample[1], -sample[2]};
    const int lane = project_lane(context, position);
    if (!right_side) {
        for (int index = lane; index >= 0; --index)
            at<float>(context, index * 4) = float(number(penalty) + at<float>(context, index * 4));
    } else {
        for (int index = lane; index < 40; ++index)
            at<float>(context, index * 4) = float(number(penalty) + at<float>(context, index * 4));
    }
    return 1;
}

Dword corridor_point_on_road(void *context, const float *point) {
    const int sample = nearest_sample(context, point);
    float left[3] = {}, right[3] = {}, lateral[3] = {}, forward[3] = {}, center[3] = {};
    corridor_sample_edges(context, sample, left, right, lateral, forward, center);
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(point[axis]) - center[axis]);
    const float projection = float(hardware::dot_product(delta, lateral));
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(left[axis]) - center[axis]);
    const auto left_width = hardware::vector_length(delta);
    if (left_width < number(projection))
        return 0;
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(right[axis]) - center[axis]);
    const auto right_width = hardware::vector_length(delta);
    return !(number(projection) < -right_width);
}
} // namespace nfs::game::runtime::ai::cars
