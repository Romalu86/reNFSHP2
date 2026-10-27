#include "car_initialization_runtime.hpp"
namespace nfs::game::world::vehicle_setup {

void initialize_body_inertia(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    float pitch_scale, roll_scale;
    switch (at<Dword>(car, 552)) {
    case 1:
    case 3:
    case 7:
        pitch_scale = .6f;
        roll_scale = .7f;
        break;
    case 4:
    case 5:
        pitch_scale = .65f;
        roll_scale = .85f;
        break;
    case 6:
        pitch_scale = .65f;
        roll_scale = .8f;
        break;
    default:
        pitch_scale = .83f;
        roll_scale = .84f;
        break;
    }
    void *body = at<void *>(car, 1768);
    const float x2 = float(E(at<float>(body, 120)) * at<float>(body, 120) * 4.f);
    const float y2 = float(E(at<float>(body, 124)) * at<float>(body, 124) * 4.f);
    const auto z2 = E(at<float>(body, 128)) * at<float>(body, 128) * 4.f;
    const auto pitch = (E(y2) + z2) * at<float>(body, 32) * pitch_scale * .25f;
    at<float>(car, 128) = float(pitch);
    const auto yaw = (z2 + x2) * at<float>(body, 32) * at<float>(car, 612) * .2f;
    at<float>(car, 132) = float(yaw);
    const auto roll = (E(y2) + x2) * at<float>(body, 32) * roll_scale * 0.3333333432674407958984375f;
    at<float>(car, 136) = float(roll);
    at<float>(car, 140) = float(E(1) / E(at<float>(car, 128)));
    at<float>(car, 144) = float(E(1) / E(at<float>(car, 132)));
    at<float>(car, 148) = float(E(1) / roll);
}

void initialize_gear_speeds(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    const float engine_speed = float(I(at<int>(car, 660)) * 0.01666666753590106964111328125f);
    const auto sidewall = I(at<int>(car, 1192)) * .01f * (I(at<int>(car, 1188)) * .001f);
    const float circumference =
        float((sidewall + sidewall + I(at<int>(car, 1196)) * .0254f) * 3.1415927410125732421875f);
    for (int gear = 0; gear < at<int>(car, 780); ++gear) {
        float final_ratio = at<float>(car, 888);
        if (final_ratio < at<float>(car, 884))
            final_ratio = at<float>(car, 884);
        const auto ratio = E(final_ratio) * at<float>(car, 788 + Dword(gear) * 4);
        float speed;
        if (double(ratio) != 0)
            speed = float(E(engine_speed) / ratio * circumference);
        else
            speed = 0;
        const auto absolute = speed < 0 ? -E(speed) : E(speed);
        const auto inverse_speed = E(0) < absolute ? I(at<int>(car, 660)) / E(speed) : E(0);
        at<float>(car, 184 + Dword(gear) * 4) = float(inverse_speed);
    }
    const int gear_count = at<int>(car, 780);
    if (gear_count < 8)
        for (int gear = gear_count; gear < 8; ++gear)
            at<Dword>(car, 184 + Dword(gear) * 4) = 0;
    const float reverse = at<float>(car, 184);
    at<Dword>(car, 188) = 0;
    at<float>(car, 184) = -(reverse < 0 ? -reverse : reverse);
}

void initialize_drive_weights(void *car) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    at<float>(car, 16) = float(E(1) / (E(at<float>(car, 768)) * 0.18076808750629425048828125f));
    float final_ratio = at<float>(car, 888);
    if (final_ratio < at<float>(car, 884))
        final_ratio = at<float>(car, 884);
    at<float>(car, 248) = float(E(final_ratio) * 0.1518000066280364990234375f);
    for (int gear = 0; gear < at<int>(car, 780); ++gear)
        at<float>(car, 216 + Dword(gear) * 4) =
            float(E(at<float>(car, 788 + Dword(gear) * 4)) * 0.0139712393283843994140625f);
    at<Dword>(car, 220) = 0;
    int driven = 0;
    for (unsigned wheel = 0; wheel < 4; ++wheel)
        if (at<Byte>(car, 2169 + wheel * 296))
            ++driven;
    if (!driven)
        at<Byte>(car, 3057) = at<Byte>(car, 2761) = 1;
    const auto front = E(at<float>(car, 560)) * .5f;
    at<float>(car, 2152) = at<float>(car, 2448) = float(front);
    at<float>(car, 3040) = at<float>(car, 2744) = float(E(.5f) - front);
}

void build_gear_force_curve(void *car, int gear, float *forces) {
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    auto I = [&](int value) { return Extended::integer(value, format); };
    const auto sidewall = I(at<int>(car, 1192)) * .01f * (I(at<int>(car, 1188)) * .001f);
    const int rim = at<int>(car, 1196);
    const float wheel_radius = float((sidewall + sidewall + I(rim) * .0254f) * .5f);
    for (int speed = 0; speed < 135; ++speed) {
        const auto retained_rpm = I(speed) * at<float>(car, 184 + Dword(gear) * 4);
        float rpm;
        if (retained_rpm < E(0))
            rpm = 0;
        else if (!(retained_rpm < E(12000)) && double(retained_rpm) >= 12000)
            rpm = 12000;
        else
            rpm = float(retained_rpm);
        if (I(at<int>(car, 660)) < E(rpm) || E(rpm) < I(at<int>(car, 656))) {
            forces[speed] = 0;
            continue;
        }
        const float gear_ratio = at<float>(car, 788 + Dword(gear) * 4);
        float final_ratio = at<float>(car, 888);
        if (final_ratio < at<float>(car, 884))
            final_ratio = at<float>(car, 884);
        const float efficiency = at<float>(car, 820 + Dword(gear) * 4);
        const auto rounded = E(rpm).round_to_integer();
        const int engine_rpm =
            rounded < INT32_MIN || rounded > INT32_MAX ? INT32_MIN : int(rounded);
        const auto torque = engine_torque(bytes(car) + 656, engine_rpm);
        forces[speed] = float(E(efficiency) * final_ratio * gear_ratio / E(wheel_radius) * torque);
    }
}
} // namespace nfs::game::world::vehicle_setup
