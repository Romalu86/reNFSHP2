#include "car_corridor_x87_private.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
namespace nfs::game::runtime::ai::cars {

Dword corridor_apply_body(void *context, void *body, float penalty, void *entity) {
    auto E = [](float value) { return static_cast<long double>(value); };
    float position[3] = {at<float>(body, 680), 0.f, at<float>(body, 688)};
    float delta[3] = {float(E(position[0]) - at<float>(context, 644)), -at<float>(context, 648),
                      float(E(position[2]) - at<float>(context, 652))};
    const auto projection =
        hardware::dot_product(
                                  delta, reinterpret_cast<const float *>(bytes(context) + 668));
    const float distance = float(projection);
    if (projection <= E(0))
        return 0;
    void *own_body = at<void *>(at<void *>(context, 640), 1768);
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(E(at<float>(own_body, 8 + axis * 4)) - at<float>(body, 8 + axis * 4));
    const auto closing =
        hardware::dot_product(
                                  delta, reinterpret_cast<const float *>(bytes(context) + 668));
    const float closing_speed = float(closing);
    if (closing <= E(0))
        return 0;
    const auto quotient = E(distance) / E(closing_speed);
    float time = float(quotient);
    if (quotient <= E(0))
        return 0;
    if (time > at<float>(context, 696))
        return 0;
    penalty = float((E(1) - E(time) / E(at<float>(context, 696))) * penalty);
    if (at<float>(context, 696) < time)
        time = at<float>(context, 696);
    physics::copy_triplet(delta, position);
    float relative[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        relative[axis] = float(E(delta[axis]) - at<float>(context, 644 + axis * 4));
    int minimum = corridor_detail::lane_index(
        hardware::dot_product(relative, reinterpret_cast<const float *>(bytes(context) + 680)));
    int maximum = minimum;
    float corners[4][3];
    std::memcpy(corners, bytes(body) + 404, sizeof(corners));
    for (unsigned corner = 0; corner < 4; ++corner) {
        float local[3], transformed[3] = {0, 0, 0}, copied[3];
        std::memcpy(local, corners[corner], 12);
        matrices::transform_vectors(1, local, reinterpret_cast<const float *>(bytes(body) + 692),
                                    transformed);
        for (unsigned axis = 0; axis < 3; ++axis)
            local[axis] = float(E(transformed[axis]) + position[axis]);
        physics::copy_triplet(copied, local);
        for (unsigned axis = 0; axis < 3; ++axis)
            relative[axis] = float(E(copied[axis]) - at<float>(context, 644 + axis * 4));
        const int lane = corridor_detail::lane_index(hardware::dot_product(
            relative, reinterpret_cast<const float *>(bytes(context) + 680)));
        if (lane < minimum)
            minimum = lane;
        if (maximum < lane)
            maximum = lane;
    }
    if (maximum < minimum)
        std::swap(minimum, maximum);
    minimum = audio::vehicle::clamp_value(std::int32_t(Dword(minimum) - 3), 0, 39);
    maximum = audio::vehicle::clamp_value(std::int32_t(Dword(maximum) + 3), 0, 39);
    for (int lane = minimum; lane <= maximum; ++lane) {
        const Dword offset = Dword(lane) * 4;
        if (distance < at<float>(context, 160 + offset)) {
            auto &cost = at<float>(context, offset);
            const auto product = E(penalty) * cost;
            if (E(0) < product)
                cost = float(E(penalty) + cost);
            else
                cost = penalty;
            at<float>(context, 160 + offset) = distance;
            if (penalty < 0.f)
                at<float>(context, 480 + offset) = 0.f < closing_speed ? closing_speed : 0.f;
            auto &previous_time = at<float>(context, 320 + offset);
            if (!(previous_time < time))
                previous_time = time;
        }
    }
    if (entity && minimum <= at<int>(context, 796) && at<int>(context, 796) <= maximum &&
        closing_speed > 0.f && distance > 0.f) {
        void *other_car = physics::vehicle_car(body);
        if (other_car)
            invoke(entity, 38, other_car, closing_speed, distance);
    }
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
