#include "car_corridor_x87_private.hpp"
namespace nfs::game::runtime::values {

VectorValue *subtract_vector_value(VectorValue *output, const VectorValue *first,
                                   const float *second) {
    const auto z = static_cast<long double>(first->z) - second[2];
    const auto y = static_cast<long double>(first->y) - second[1];
    const auto x = static_cast<long double>(first->x) - second[0];
    auto &methods = *reinterpret_cast<const gui::Destroy *volatile *>(&output->methods);
    methods = gui::base_methods;
    output->x = float(x);
    methods = vector_methods;
    output->y = float(y);
    output->z = float(z);
    return output;
}
} // namespace nfs::game::runtime::values
namespace nfs::game::runtime::ai::cars {
using namespace corridor_detail;

void copy_car_path(void *destination, const void *source) {
    at<Dword>(destination, 0) = at<Dword>(source, 0);
    for (unsigned base : {4u, 16u, 28u}) {
        at<Dword>(destination, base) = at<Dword>(source, base);
        const Dword second = at<Dword>(source, base + 4), third = at<Dword>(source, base + 8);
        at<Dword>(destination, base + 4) = second;
        at<Dword>(destination, base + 8) = third;
    }
    at<Dword>(destination, 40) = at<Dword>(source, 40);
    at<Dword>(destination, 44) = at<Dword>(source, 44);
    const Dword last = at<Dword>(source, 48);
    const auto count = at<std::int16_t>(source, 84);
    at<Dword>(destination, 48) = last;
    at<std::int16_t>(destination, 84) = count;
    if (count != 0)
        for (unsigned index = 0; index < at<std::uint16_t>(destination, 84); ++index)
            at<Dword>(destination, 52 + index * 4) = at<Dword>(source, 52 + index * 4);
}

void *car_path(void *entity) {
    return bytes(entity) + 220;
}

void *car_physical_object(void *entity) {
    return at<void *>(entity, 368);
}

void *construct_car_corridor(void *context) {
    for (unsigned offset = 640; offset <= 700; offset += 4)
        at<Dword>(context, offset) = 0;
    at<Dword>(context, 704) = 20;
    lifecycle::pending::construct_population_path(bytes(context) + 708);
    for (unsigned offset = 796; offset <= 808; offset += 4)
        at<Dword>(context, offset) = 0;
    for (unsigned base : {0u, 160u, 480u, 320u})
        for (unsigned index = 0; index < 40; ++index)
            at<Dword>(context, base + index * 4) = 0;
    return context;
}

Dword corridor_sample_edges(void *context, int sample, float *left, float *right, float *lateral,
                           float *forward, float *center) {
    float pose[21], next_pose[21];
    lifecycle::sample_path(bytes(context) + 708, sample, pose);
    values::VectorValue term, sum;
    term.methods = gui::base_methods;
    term.methods = values::vector_methods;
    values::assign_vector_value(&term, pose + 15);
    values::sum_vector_value(&sum, &term, pose);
    physics::copy_triplet(left, &sum.x);
    sum.methods = gui::base_methods;
    left[2] = -left[2];
    term.methods = gui::base_methods;
    term.methods = values::vector_methods;
    values::assign_vector_value(&term, pose + 12);
    values::sum_vector_value(&sum, &term, pose);
    physics::copy_triplet(right, &sum.x);
    sum.methods = gui::base_methods;
    right[2] = -right[2];
    if (at<int>(context, 728) < 0) {
        const float x = right[0], y = right[1], z = right[2];
        physics::copy_triplet(right, left);
        left[0] = x;
        left[1] = y;
        left[2] = z;
    }
    term.methods = gui::base_methods;
    term.methods = values::vector_methods;
    values::assign_vector_value(&term, pose);
    values::sum_vector_value(&sum, &term, pose + 18);
    physics::copy_triplet(center, &sum.x);
    sum.methods = gui::base_methods;
    term.methods = gui::base_methods;
    center[2] = -center[2];
    lifecycle::sample_path(bytes(context) + 708, std::int32_t(Dword(sample) + 1), next_pose);
    values::VectorValue current_center, next_center, difference;
    term.methods = values::vector_methods;
    values::assign_vector_value(&term, pose);
    values::sum_vector_value(&current_center, &term, pose + 18);
    term.methods = gui::base_methods;
    term.methods = values::vector_methods;
    values::assign_vector_value(&term, next_pose);
    values::sum_vector_value(&next_center, &term, next_pose + 18);
    values::subtract_vector_value(&difference, &next_center, &current_center.x);
    physics::copy_triplet(forward, &difference.x);
    difference.methods = gui::base_methods;
    next_center.methods = gui::base_methods;
    term.methods = gui::base_methods;
    current_center.methods = gui::base_methods;
    const auto length = physics::pending::normalize_collision_entry(forward);
    if (length == 0.f)
        physics::copy_triplet(forward, pose + 3);
    forward[2] = -forward[2];
    lateral[0] = forward[2];
    lateral[1] = 0.f;
    lateral[2] = -forward[0];
    return 1;
}

void corridor_blend_axes(void *context) {
    float edge[3] = {}, left_lateral[3] = {}, left_forward[3] = {}, left_center[3] = {};
    corridor_sample_edges(context, at<float>(context, 804) > 0.f, edge, edge, left_lateral, left_forward,
                          left_center);
    float right_lateral[3] = {}, right_forward[3] = {}, right_center[3] = {};
    corridor_sample_edges(context, !(0.f < at<float>(context, 804)) ? -1 : 0, edge, edge, right_lateral,
                          right_forward, right_center);
    float delta[3];
    for (unsigned axis = 3; axis-- > 0;)
        delta[axis] = float(number(left_center[axis]) - right_center[axis]);
    const auto half_width = hardware::vector_length(delta) * 0.5f;
    if (half_width != number(0)) {
        auto ratio = number(at<float>(context, 804));
        if (ratio < number(0))
            ratio = -ratio;
        ratio = ratio / half_width;
        if (!(ratio < number(1)))
            ratio = number(1);
        const float first_weight = float(number(1) - ratio);
        const float second_weight = float(number(1) - first_weight);
        const bool positive_offset = 0.f < at<float>(context, 804);
        const float left_weight = positive_offset ? second_weight : first_weight;
        const float right_weight = positive_offset ? first_weight : second_weight;
        float weighted_right[3], weighted_left[3];
        for (unsigned axis = 3; axis-- > 0;)
            weighted_right[axis] = float(number(right_lateral[axis]) * right_weight);
        for (unsigned axis = 3; axis-- > 0;)
            weighted_left[axis] = float(number(left_lateral[axis]) * left_weight);
        for (unsigned axis = 3; axis-- > 0;)
            left_lateral[axis] = float(number(weighted_right[axis]) + weighted_left[axis]);
        for (unsigned axis = 3; axis-- > 0;)
            weighted_right[axis] = float(number(right_forward[axis]) * right_weight);
        for (unsigned axis = 3; axis-- > 0;)
            weighted_left[axis] = float(number(left_forward[axis]) * left_weight);
        if (positive_offset) {
            for (unsigned axis = 3; axis-- > 0;)
                left_forward[axis] = float(number(weighted_right[axis]) + weighted_left[axis]);
        } else {
            for (unsigned axis = 0; axis < 3; ++axis)
                left_forward[axis] = float(number(weighted_left[axis]) + weighted_right[axis]);
        }
        physics::pending::normalize_collision_entry(left_lateral);
        physics::pending::normalize_collision_entry(left_forward);
    }
    physics::copy_triplet(vector(context, 668), left_forward);
    physics::copy_triplet(vector(context, 680), left_lateral);
}
} // namespace nfs::game::runtime::ai::cars
