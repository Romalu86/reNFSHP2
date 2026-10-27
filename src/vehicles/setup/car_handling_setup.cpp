#include "car_initialization_runtime.hpp"
#include "../../physics/vehicles/car_construction_runtime.hpp"
namespace nfs::game::runtime::ai::entities::pending {
kernel::numeric::Extended direction_turns(float, float);
}
namespace nfs::game::world::vehicle_setup {

void initialize_cornering_balance(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    const float suspension = at<float>(car, 348);
    alignas(4) Byte wheel[296];
    runtime::physics::pending::construct_ground_contact(wheel + 48);
    runtime::physics::construction::initialize_wheel_state(wheel, nullptr, nullptr, 0, nullptr);
    const float axle_offset = float(E(at<float>(car, 548)) * at<float>(car, 528));
    at<Dword>(wheel, 24) = 0;
    auto width_factor = I(at<int>(car, 912)) * 0.0046511627733707427978515625f;
    if (width_factor < E(.9f))
        width_factor = E(.9f);
    else if (E(1.5f) < width_factor)
        width_factor = E(1.5f);
    at<float>(wheel, 268) = float(width_factor);
    const float angle =
        float(runtime::ai::entities::pending::direction_turns(at<float>(car, 528), 33));
    at<float>(wheel, 20) = angle;
    const float front_load = float(E(at<float>(car, 520)) / E(at<float>(car, 528)) *
                                   9.81000041961669921875f * axle_offset * .5f);
    at<float>(wheel, 232) = front_load;
    const float front_lateral =
        float(tire_lateral_force(bytes(car) + 912, wheel) * at<float>(car, 948));
    const float front_moment = float(tire_aligning_moment(bytes(car) + 912, wheel));
    const auto inverse_angle = E(1) / (E(at<float>(wheel, 20)) * 360.f);
    const auto normalized_lateral = E(front_lateral) * inverse_angle;
    const float normalized_front_moment = float(E(front_moment) * inverse_angle);
    const float front_couple =
        float((E(at<float>(car, 616)) * suspension +
               E(at<float>(car, 544)) * 0.454545438289642333984375f * (E(front_load) + front_load)) /
              E(9.81000041961669921875f) * 2.9309256076812744140625f / E(at<float>(car, 532)));
    const auto inverse_front_lateral = E(1) / normalized_lateral;
    const float front_load_ratio = float(E(front_load) * inverse_front_lateral);
    const float front_term = float(inverse_front_lateral * front_couple * front_couple *
                                   0.0000970500041148625314235687255859375f * front_load_ratio);
    at<float>(wheel, 20) = angle;
    width_factor = I(at<int>(car, 1188)) * 0.0046511627733707427978515625f;
    if (width_factor < E(.9f))
        width_factor = E(.9f);
    else if (E(1.5f) < width_factor)
        width_factor = E(1.5f);
    at<float>(wheel, 268) = float(width_factor);
    const auto rear_offset = (E(1) - at<float>(car, 548)) * at<float>(car, 528);
    const float rear_load = float(E(at<float>(car, 520)) / E(at<float>(car, 528)) *
                                  9.81000041961669921875f * rear_offset * .5f);
    at<float>(wheel, 232) = rear_load;
    const float rear_lateral =
        float(tire_lateral_force(bytes(car) + 1188, wheel) * at<float>(car, 1224));
    const auto rear_moment = tire_aligning_moment(bytes(car) + 1188, wheel);
    const auto rear_inverse_angle = E(1) / (E(at<float>(wheel, 20)) * 360.f);
    const float normalized_rear_moment = float(rear_moment * rear_inverse_angle);
    const auto rear_couple =
        (E(at<float>(car, 620)) * suspension +
         E(at<float>(car, 544)) * 0.454545438289642333984375f * (E(rear_load) + rear_load)) /
        E(9.81000041961669921875f) * 2.9309256076812744140625f / E(at<float>(car, 536));
    const float rear_lateral_stored = float(E(rear_lateral) * rear_inverse_angle);
    const auto inverse_rear_lateral = E(1) / E(rear_lateral_stored);
    const float rear_load_ratio = float(E(rear_load) * inverse_rear_lateral);
    const auto rear_term =
        rear_couple * rear_couple * inverse_rear_lateral * 0.0000970500041148625314235687255859375f * rear_load_ratio;
    auto geometry = E(0);
    if (normalized_front_moment != 0 && normalized_rear_moment != 0) {
        geometry = (E(normalized_rear_moment) + normalized_front_moment) /
                   (E(normalized_rear_moment) * normalized_front_moment) /
                   E(at<float>(car, 528)) * 9.81000041961669921875f;
        if (geometry < E(0))
            geometry = -geometry;
    }
    auto balance = (E(front_load_ratio) - rear_load_ratio) + (E(front_term) - rear_term) + geometry;
    if (balance < E(-10))
        balance = E(-10);
    else if (E(10) < balance)
        balance = E(10);
    at<float>(car, 256) = float(balance);
}

Dword initialize_derived_parameters(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    using runtime::ai::entities::pending::direction_turns;
    at<float>(car, 104) = float(E(at<float>(car, 528)) * .5f);
    at<float>(car, 108) = float(E(at<float>(car, 528)) * -.5f);
    initialize_body_inertia(car);
    at<float>(car, 264) = float(direction_turns(at<float>(car, 652), at<float>(car, 528)));
    at<float>(car, 268) = float(direction_turns(at<float>(car, 648), at<float>(car, 528)));
    float travel = at<float>(car, 652);
    if (travel < at<float>(car, 648))
        travel = at<float>(car, 648);
    const float half_width = at<float>(at<void *>(car, 1768), 120);
    at<float>(car, 272) = float(direction_turns(travel, float(E(half_width) + half_width)));
    initialize_gear_speeds(car);
    for (int gear = 0; gear < at<int>(car, 780); ++gear) {
        const unsigned field = 184 + Dword(gear) * 4;
        auto efficiency = E(at<float>(car, field + 636));
        if (efficiency < E(0))
            efficiency = E(0);
        else if (E(1) < efficiency)
            efficiency = E(1);
        at<float>(car, field + 636) = float(efficiency);
        const float rpm_per_speed = at<float>(car, field);
        at<float>(car, field + 92) = rpm_per_speed == 0 ? .01f : float(E(1) / E(rpm_per_speed));
        float final_ratio = at<float>(car, 888);
        if (final_ratio < at<float>(car, 884))
            final_ratio = at<float>(car, 884);
        at<float>(car, field - 32) =
            float(E(at<float>(car, field + 604)) * final_ratio * efficiency);
        at<float>(car, field + 124) =
            float(E(1) / (I(at<int>(car, 660)) * at<float>(car, field + 92)));
    }
    at<float>(car, 124) = float(direction_turns(at<float>(car, 528), at<float>(car, 588)));
    auto length_difference =
        (E(at<float>(car, 104)) - at<float>(car, 108)) - at<float>(car, 528);
    if (length_difference < E(0))
        length_difference = -length_difference;
    if (E(.5f) < length_difference)
        return 0;
    void *body = at<void *>(car, 1768);
    const auto rear_distance = (E(1) - at<float>(car, 548)) * at<float>(car, 528);
    at<float>(car, 112) = float(rear_distance);
    at<float>(car, 116) = float(E(at<float>(car, 548)) * at<float>(car, 528));
    at<float>(body, 52) = float(E(at<float>(car, 104)) - rear_distance);
    at<float>(body, 48) = float(E(at<float>(car, 544)) - at<float>(body, 124));
    at<float>(car, 120) = float(E(at<float>(car, 540)) * at<float>(car, 532) * .8f);
    const auto sprung_mass = E(at<float>(car, 520)) - at<float>(car, 524);
    const float spring_sum = float(E(at<float>(car, 620)) + at<float>(car, 616));
    at<float>(car, 340) =
        float(E(spring_sum) - E(at<float>(car, 544)) * 0.454545438289642333984375f * 9.81000041961669921875f *
                                  sprung_mass * 0.01745329238474369049072265625f);
    at<float>(car, 344) =
        float(E(spring_sum) - E(at<float>(car, 544)) * 0.318181812763214111328125f * 9.81000041961669921875f *
                                  sprung_mass * 0.01745329238474369049072265625f);
    const auto height_scale = E(at<float>(car, 544)) * 0.590909063816070556640625f;
    at<float>(car, 348) =
        float((E(spring_sum) - sprung_mass * height_scale * 9.81000041961669921875f * 0.01745329238474369049072265625f) /
              E(at<float>(car, 520)) * height_scale * 9.81000041961669921875f);
    initialize_brake_balance(car);
    initialize_cornering_balance(car);
    initialize_top_speed(car);
    return 1;
}
} // namespace nfs::game::world::vehicle_setup
