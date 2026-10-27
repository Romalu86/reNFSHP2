#include "car_corridor_x87_private.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

Dword corridor_restrict_side(void *context, void *car, float penalty, float width_margin,
                            float length_margin) {
    auto E = [](float value) { return static_cast<long double>(value); };
    void *body = at<void *>(car, 1768);
    float delta[3];
    const auto dz = E(at<float>(body, 688)) - at<float>(context, 652);
    const auto dy = E(at<float>(body, 684)) - at<float>(context, 648);
    const auto dx = E(at<float>(body, 680)) - at<float>(context, 644);
    delta[0] = float(dx);
    delta[1] = float(dy);
    delta[2] = float(dz);
    float relative[3] = {0, 0, 0};
    matrices::transform_vectors(1, delta,
                                reinterpret_cast<const float *>(
                                    bytes(at<void *>(at<void *>(context, 640), 1768)) + 72),
                                relative);
    std::memcpy(delta, relative, 12);
    float velocity[3];
    std::memcpy(velocity, bytes(body) + 8, 12);
    static_cast<void>(hardware::dot_product(
        velocity, reinterpret_cast<const float *>(bytes(context) + 668)));
    void *own_body = at<void *>(at<void *>(context, 640), 1768);
    values::VectorValue own;
    own.methods = values::vector_methods;
    own.x = at<float>(own_body, 120);
    own.y = at<float>(own_body, 124);
    own.z = at<float>(own_body, 128);
    values::VectorValue extents;
    values::sum_vector_value(&extents, &own,
                             reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 120));
    const auto longitudinal_extent = E(extents.z) + length_margin;
    if (longitudinal_extent < E(relative[2]) || E(relative[2]) < -longitudinal_extent)
        return 0;
    const auto lateral_extent = E(extents.x) + width_margin;
    const auto lateral = relative[0] < 0 ? -E(relative[0]) : E(relative[0]);
    if (lateral_extent < lateral)
        return 0;
    if (!(relative[0] < 0)) {
        for (int lane = std::int32_t(at<Dword>(context, 796) + 1); lane < 40; ++lane)
            at<float>(context, Dword(lane) * 4) =
                float(E(penalty) + at<float>(context, Dword(lane) * 4));
    } else {
        for (int lane = 0; lane < at<int>(context, 796); ++lane)
            at<float>(context, Dword(lane) * 4) =
                float(E(penalty) + at<float>(context, Dword(lane) * 4));
    }
    return 1;
}

Dword corridor_restrict_behind(void *context, void *car, float penalty, float width_margin,
                              float length_margin) {
    auto E = [](float value) { return static_cast<long double>(value); };
    float position[3], delta[3];
    void *body = at<void *>(car, 1768);
    physics::copy_triplet(position, reinterpret_cast<const float *>(bytes(body) + 680));
    const float extents = float(E(at<float>(at<void *>(at<void *>(context, 640), 1768), 128)) +
                                at<float>(body, 128));
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(E(position[axis]) - at<float>(context, 644 + axis * 4));
    const auto longitudinal =
        hardware::dot_product(
                                  delta, reinterpret_cast<const float *>(bytes(context) + 668)) +
        extents;
    const float stored_longitudinal = float(longitudinal);
    if (E(0) <= longitudinal)
        return 0;
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(E(position[axis]) - at<float>(context, 644 + axis * 4));
    auto lateral =
        hardware::dot_product(
                                  delta, reinterpret_cast<const float *>(bytes(context) + 680));
    if (-stored_longitudinal > length_margin)
        return 0;
    if (lateral < E(0))
        lateral = -lateral;
    if (E(width_margin) < lateral)
        return 0;
    values::VectorValue own, extents_value;
    own.methods = gui::base_methods;
    own.methods = values::vector_methods;
    values::assign_vector_value(&own,
                                reinterpret_cast<const float *>(
                                    bytes(at<void *>(at<void *>(context, 640), 1768)) + 120));
    values::sum_vector_value(&extents_value, &own,
                             reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 120));
    float summed[3];
    physics::copy_triplet(summed, &extents_value.x);
    values::destroy_vector(&extents_value);
    if (penalty == 0.f)
        return 0;
    physics::copy_triplet(delta, position);
    float difference[3];
    values::difference_triplets(difference, delta,
                                reinterpret_cast<const float *>(bytes(context) + 644));
    const auto center = hardware::dot_product(
        difference, reinterpret_cast<const float *>(bytes(context) + 680));
    int minimum = corridor_detail::lane_index(center);
    int maximum = minimum;
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        float local[3], transformed[3] = {0, 0, 0}, copied[3];
        const auto offset = 2224 + wheel * 296;
        std::memcpy(local, bytes(car) + offset, 12);
        matrices::transform_vectors(
            1, local, reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 692),
            transformed);
        for (unsigned axis = 0; axis < 3; ++axis)
            local[axis] = float(E(transformed[axis]) + position[axis]);
        physics::copy_triplet(copied, local);
        values::difference_triplets(difference, copied,
                                    reinterpret_cast<const float *>(bytes(context) + 644));
        const int lane = corridor_detail::lane_index(hardware::dot_product(
            difference, reinterpret_cast<const float *>(bytes(context) + 680)));
        if (lane < minimum)
            minimum = lane;
        if (maximum < lane)
            maximum = lane;
    }
    if (maximum < minimum)
        std::swap(minimum, maximum);
    minimum = audio::vehicle::clamp_value(std::int32_t(Dword(minimum) - 3), 0, 39);
    maximum = audio::vehicle::clamp_value(std::int32_t(Dword(maximum) + 3), 0, 39);
    for (int lane = minimum; lane <= maximum; ++lane)
        at<float>(context, Dword(lane) * 4) = float(E(penalty) + at<float>(context, Dword(lane) * 4));
    return 1;
}

Dword corridor_center_contact(void *context, float *speed, float *distance) {
    const Dword offset = Dword(at<int>(context, 796)) * 4;
    if (!(at<float>(context, 480 + offset) > 0.f) ||
        !(at<float>(context, 160 + offset) > 0.f))
        return 0;
    const Dword distance_bits = at<Dword>(context, 160 + offset);
    const Dword speed_bits = at<Dword>(context, 480 + offset);
    at<Dword>(distance, 0) = distance_bits;
    at<Dword>(speed, 0) = speed_bits;
    return 1;
}

Dword corridor_apply_car(void *context, void *car, float penalty, float margin, void *entity) {
    auto E = [](float value) { return static_cast<long double>(value); };
    void *body = at<void *>(car, 1768);
    float position[3] = {at<float>(body, 680), at<float>(body, 684), at<float>(body, 688)};
    position[1] = 0.f;
    if (margin <= 0.f)
        return 0;
    const float own_length = at<float>(at<void *>(at<void *>(context, 640), 1768), 128);
    const float other_length = at<float>(body, 128);
    margin = float(E(own_length) + other_length + margin);
    float delta[3] = {float(E(position[0]) - at<float>(context, 644)), -at<float>(context, 648),
                      float(E(position[2]) - at<float>(context, 652))};
    float distance =
        float(hardware::dot_product(delta, reinterpret_cast<const float *>(bytes(context) + 668)));
    if (distance <= -at<float>(at<void *>(at<void *>(context, 640), 1768), 128))
        return 0;
    if (margin < distance)
        return 0;
    if (distance < 0.f)
        distance = 0.f;
    if (penalty == 0.f)
        return 0;
    physics::copy_triplet(delta, position);
    float difference[3];
    values::difference_triplets(difference, delta,
                                reinterpret_cast<const float *>(bytes(context) + 644));
    int minimum = corridor_detail::lane_index(
        hardware::dot_product(difference, reinterpret_cast<const float *>(bytes(context) + 680)));
    int maximum = minimum;
    for (unsigned wheel = 0; wheel < 4; ++wheel) {
        float local[3], transformed[3] = {0, 0, 0}, copied[3];
        std::memcpy(local, bytes(car) + 2224 + wheel * 296, 12);
        matrices::transform_vectors(
            1, local, reinterpret_cast<const float *>(bytes(at<void *>(car, 1768)) + 692),
            transformed);
        for (unsigned axis = 0; axis < 3; ++axis)
            local[axis] = float(E(transformed[axis]) + position[axis]);
        physics::copy_triplet(copied, local);
        values::difference_triplets(difference, copied,
                                    reinterpret_cast<const float *>(bytes(context) + 644));
        const int lane = corridor_detail::lane_index(hardware::dot_product(
            difference, reinterpret_cast<const float *>(bytes(context) + 680)));
        if (lane < minimum)
            minimum = lane;
        if (maximum < lane)
            maximum = lane;
    }
    if (maximum < minimum)
        std::swap(minimum, maximum);
    minimum = audio::vehicle::clamp_value(std::int32_t(Dword(minimum) - 2), 0, 39);
    maximum = audio::vehicle::clamp_value(std::int32_t(Dword(maximum) + 2), 0, 39);
    for (int lane = minimum; lane <= maximum; ++lane)
        at<float>(context, Dword(lane) * 4) = float(E(penalty) + at<float>(context, Dword(lane) * 4));
    if (entity && minimum <= at<int>(context, 796) && at<int>(context, 796) <= maximum &&
        distance > 0.f && invoke<Byte>(entity, 39, car, distance))
        at<float>(context, 808) =
            float(static_cast<long double>(invoke<long double>(at<void *>(car, 7988), 22)) *
                  0.95f);
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
