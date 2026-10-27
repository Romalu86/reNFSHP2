#include "car_corridor_x87_private.hpp"
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::runtime::ai::cars {

const float *path_sample_position(void *path, int offset) {
    void *reference;
    int sample;
    lifecycle::locate_path_sample(path, offset, &reference, &sample);
    return static_cast<const float *>(dependencies::section_node(at<void *>(reference, 8), sample));
}

int path_minimum_sample(void *path, int lower_bound) {
    Dword extent;
    if (at<int>(path, 20) > 0) {
        extent = at<Dword>(path, 16);
        if (at<Dword>(path, 8))
            extent += at<Dword>(at<void *>(path, 12), 36);
    } else {
        const Dword current = at<Dword>(path, 20) ? at<Dword>(at<void *>(path, 24), 36) : 0;
        const Dword previous = at<Dword>(path, 8) ? at<Dword>(at<void *>(path, 12), 36) : 0;
        extent = previous - at<Dword>(path, 16) + current;
    }
    int result = std::int32_t(Dword(0) - extent);
    if (lower_bound && lower_bound > result)
        result = lower_bound;
    return result;
}

Dword corridor_apply_spike(void *context, void *spike, float penalty) {
    auto E = [](float value) { return static_cast<long double>(value); };
    void *body = invoke<void *>(entities::interface_at(spike, 3), 0);
    float position[3];
    std::memcpy(position, bytes(body) + 680, 12);
    float delta[3] = {float(E(position[0]) - at<float>(context, 644)), -at<float>(context, 648),
                      float(E(position[2]) - at<float>(context, 652))};
    position[1] = 0.f;
    const auto projected =
        hardware::dot_product(
                                  delta, reinterpret_cast<const float *>(bytes(context) + 668));
    const float distance = float(projected);
    if (projected <= E(0))
        return 0;
    float speed = at<float>(context, 692);
    if (speed < at<float>(context, 808))
        speed = at<float>(context, 808);
    if (speed <= 0.f)
        return 0;
    float time = float(E(distance) / E(speed));
    if (time <= 0.f || time > at<float>(context, 696))
        return 0;
    const float cost = float((E(1) - (E(0.75f) / E(at<float>(context, 696))) * time) * penalty);
    if (at<float>(context, 696) < time)
        time = at<float>(context, 696);
    void *path = bytes(context) + 708;
    const int first = path_minimum_sample(path, -100);
    const int end = lifecycle::path_length(path, 100);
    float closest_distance = 10000.f;
    int closest = 0;
    for (int sample = first; sample < end; ++sample) {
        float route_position[3], own[3], difference[3];
        physics::copy_triplet(route_position, path_sample_position(path, sample));
        route_position[2] = -route_position[2];
        physics::copy_triplet(own, position);
        values::difference_triplets(difference, own, route_position);
        const auto distance_to_sample = hardware::vector_length(difference);
        if (distance_to_sample < E(closest_distance)) {
            closest_distance = float(distance_to_sample);
            closest = sample;
        }
    }
    if (first < end && closest < 0)
        return 0;
    physics::copy_triplet(delta, position);
    float difference[3];
    values::difference_triplets(difference, delta,
                                reinterpret_cast<const float *>(bytes(context) + 644));
    int minimum = corridor_detail::lane_index(
        hardware::dot_product(difference, reinterpret_cast<const float *>(bytes(context) + 680)));
    int maximum = minimum;
    for (unsigned corner = 0; corner < 4; ++corner) {
        float point[3];
        physics::copy_triplet(point,
                              reinterpret_cast<const float *>(bytes(spike) + 236 + corner * 12));
        values::difference_triplets(difference, point,
                                    reinterpret_cast<const float *>(bytes(context) + 644));
        const int lane = corridor_detail::lane_index(hardware::dot_product(
            difference, reinterpret_cast<const float *>(bytes(context) + 680)));
        if (lane < minimum)
            minimum = lane;
        if (maximum < lane)
            maximum = lane;
    }
    minimum = audio::vehicle::clamp_value(std::int32_t(Dword(minimum) - 1), 0, 39);
    maximum = audio::vehicle::clamp_value(std::int32_t(Dword(maximum) + 1), 0, 39);
    if (maximum < minimum)
        std::swap(minimum, maximum);
    for (int lane = minimum; lane <= maximum; ++lane) {
        const Dword offset = Dword(lane) * 4;
        if (distance < at<float>(context, 160 + offset))
            at<float>(context, 160 + offset) = distance;
        at<float>(context, offset) = float(E(cost) + at<float>(context, offset));
        if (cost < 0.f) {
            const float own_speed = at<float>(context, 692);
            at<float>(context, 480 + offset) = 0.f < own_speed ? own_speed : 0.f;
        }
        auto &previous_time = at<float>(context, 320 + offset);
        if (!(previous_time < time))
            previous_time = time;
    }
    return 1;
}
} // namespace nfs::game::runtime::ai::cars
