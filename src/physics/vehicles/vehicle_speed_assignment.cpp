#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "car_drive_frame_x87.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstdint>
#include <cstring>

namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
// These fields retain the original byte layout. Parameter names come from
// SpeedGovernor, MinRPM, RedLine and NumberOfGears in the car data reader.
namespace car_speed_layout {
constexpr unsigned engine_angular_speed = 4;
constexpr unsigned transmission_gear = 96;
constexpr unsigned rpm_per_speed = 184;
constexpr unsigned first_gear_speed_per_rpm = 284;
constexpr unsigned speed_governor = 572;
constexpr unsigned minimum_rpm = 656;
constexpr unsigned redline_rpm = 660;
constexpr unsigned number_of_gears = 780;
constexpr unsigned requested_gear = 1582;
constexpr unsigned selected_gear = 1584;
constexpr unsigned rigid_body = 1768;
constexpr unsigned wheel_ground_speed = 1992;
constexpr unsigned wheel_tread_speed = 2100;
constexpr unsigned wheel_angular_speed = 2104;
constexpr unsigned loaded_wheel_radius = 2164;
constexpr unsigned wheel_stride = 296;
constexpr unsigned signed_speed = 3132;
constexpr unsigned local_velocity = 3160;
constexpr unsigned forward_speed = 3168;
} // namespace car_speed_layout

namespace body_speed_layout {
constexpr unsigned linear_velocity = 8;
constexpr unsigned world_basis = 692;
} // namespace body_speed_layout

constexpr unsigned wheel_count = 4;
constexpr std::int32_t first_forward_gear = 2;
constexpr float rpm_to_radians_per_second = 0.10471975803375244140625f;

long double retain_float(float value) {
    return static_cast<long double>(value);
}

std::int32_t signed_integer_bits(std::uint32_t value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

Dword float_bits(float value) {
    Dword result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
} // namespace


void set_car_speed(void *car, float speed) {
    using namespace car_speed_layout;
    if (speed < 0)
        speed = -speed;
    const float governor = at<float>(car, speed_governor);
    if (governor != 0 && !(speed < governor))
        speed = governor;

    for (unsigned wheel = 0; wheel != wheel_count; ++wheel)
        particles::lifecycle::release_wheel_skid(car, wheel);

    at<Dword>(car, forward_speed) = float_bits(speed);
    void *body = at<void *>(car, rigid_body);
    at<Dword>(car, local_velocity) = 0;
    at<Dword>(car, local_velocity + sizeof(float)) = 0;
    float velocity[3];
    matrices::transform_vectors(1, reinterpret_cast<float *>(bytes(car) + local_velocity),
                                reinterpret_cast<float *>(bytes(body) + body_speed_layout::world_basis),
                                velocity);

    // FILD followed by an explicit F32 store precedes the retained product.
    const float idle_rpm = static_cast<float>(static_cast<long double>(at<std::int32_t>(car, minimum_rpm)));
    Dword velocity_bits[3];
    std::memcpy(velocity_bits, velocity, sizeof(velocity_bits));
    at<Dword>(body, body_speed_layout::linear_velocity) = velocity_bits[0];
    at<Dword>(car, signed_speed) = at<Dword>(car, forward_speed);
    const long double idle_speed = retain_float(at<float>(car, first_gear_speed_per_rpm)) * idle_rpm;
    // Native C0 selects both less-than and unordered after this comparison.
    const bool search_forward_gears = !(idle_speed >= retain_float(speed));
    at<Dword>(body, body_speed_layout::linear_velocity + sizeof(float)) = velocity_bits[1];
    at<Dword>(body, body_speed_layout::linear_velocity + 2 * sizeof(float)) = velocity_bits[2];

    std::int32_t gear = first_forward_gear;
    if (search_forward_gears) {
        const auto last_gear = signed_integer_bits(at<Dword>(car, number_of_gears) - 1u);
        if (gear < last_gear) {
            const float maximum_rpm =
                static_cast<float>(static_cast<long double>(at<std::int32_t>(car, redline_rpm)));
            unsigned ratio_offset = rpm_per_speed + first_forward_gear * sizeof(float);
            while (gear < last_gear) {
                const long double candidate_rpm =
                    retain_float(at<float>(car, ratio_offset)) * at<float>(car, forward_speed);
                if (!(retain_float(maximum_rpm) < candidate_rpm) && retain_float(idle_rpm) < candidate_rpm)
                    break;
                ++gear;
                ratio_offset += sizeof(float);
            }
        }
    }

    at<Byte>(car, selected_gear) = static_cast<Byte>(gear);
    at<Byte>(car, requested_gear) = at<Byte>(car, selected_gear);
    const Dword selected = at<Byte>(car, selected_gear);
    at<Dword>(car, transmission_gear) = selected;
    const long double retained_rpm =
        retain_float(at<float>(car, rpm_per_speed + selected * sizeof(float))) *
        at<float>(car, signed_speed);
    const std::int32_t engine_rpm = physics::car_frame::drive::truncate_engine_rpm(&retained_rpm);
    at<float>(car, engine_angular_speed) =
        static_cast<float>(static_cast<long double>(engine_rpm) * rpm_to_radians_per_second);

    for (unsigned wheel = 0; wheel != wheel_count; ++wheel) {
        const unsigned wheel_offset = wheel * wheel_stride;
        at<float>(car, wheel_ground_speed + wheel_offset) =
            static_cast<float>(retain_float(at<float>(car, forward_speed)));
        at<float>(car, wheel_tread_speed + wheel_offset) =
            static_cast<float>(retain_float(at<float>(car, forward_speed)));
        at<float>(car, wheel_angular_speed + wheel_offset) =
            static_cast<float>(retain_float(at<float>(car, forward_speed)) /
                               retain_float(at<float>(car, loaded_wheel_radius + wheel_offset)));
    }
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
