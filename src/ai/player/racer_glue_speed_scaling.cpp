#include "../driving/ai_car_behavior.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Racer speed scaling requires the native intermediate format");

namespace nfs::game::runtime::ai::cars {
namespace {
namespace racer_glue_layout {
constexpr unsigned car = 368;
constexpr unsigned race_position = 576;
constexpr unsigned neighbor_count = 8;
constexpr unsigned neighbors = 12;
constexpr unsigned body_interface = 3;
constexpr unsigned body_active = 728;
constexpr unsigned participant = 1772;
constexpr unsigned race_tuning_system = 76;
constexpr unsigned race_rules_system = 92;
constexpr unsigned race_settings_system = 72;
constexpr unsigned race_rules_controller = 4;
constexpr unsigned lap_count = 228;
constexpr unsigned player_view_method = 11;
constexpr unsigned speed_method = 22;
constexpr unsigned racer_count_method = 23;
constexpr unsigned tuning = 84428;
// Field names follow the Racer.Glue.* keys in racetune.ini.
constexpr unsigned slowdown = 0;
constexpr unsigned slowdown_slack = 4;
constexpr unsigned spread_ahead = 8;
constexpr unsigned speedup = 12;
constexpr unsigned speedup_slack = 16;
constexpr unsigned spread_behind = 20;
constexpr unsigned close_lap_factor = 24;
constexpr unsigned falloff_ahead = 28;
constexpr unsigned falloff_behind = 32;
constexpr unsigned winner_boost = 36;
constexpr unsigned last_place_boost = 40; // Racer.Glue.looserboost in the game data.
}

long double number(float value) {
    return static_cast<long double>(value);
}

long double signed_integer(Dword value) {
    return static_cast<long double>(std::int32_t(value));
}

long double absolute_value(long double value) {
    return value < 0.0L ? -value : value;
}

long double racer_distance(const float *first, const float *second) {
    float delta[3];
    for (unsigned axis = 0; axis < 3; ++axis)
        delta[axis] = float(number(second[axis]) - first[axis]);
    const long double y = number(delta[1]);
    const long double x = number(delta[0]);
    const long double z = number(delta[2]);
    const long double squared_x = x * x;
    const long double squared_y = y * y;
    const long double squared_xy = squared_x + squared_y;
    const long double squared_z = z * z;
    return __builtin_sqrtl(squared_xy + squared_z);
}

float apply_glue_falloff(float distance, float range, const float *exponent, float gain) {
    const long double fraction = number(distance) / range;
    const long double exponent_value = number(*exponent);
    // The caller loads the exponent; the CRT bridge stores base, then exponent.
    const double stored_fraction = static_cast<double>(fraction);
    const double stored_exponent = static_cast<double>(exponent_value);
    const long double falloff = physics::legacy::power_retained(stored_fraction, stored_exponent);
    return float(falloff * gain);
}
}

// The virtual result stays retained until its caller stores it.
long double __fastcall racer_glue_speed_factor(void *entity, void *) {
    using namespace racer_glue_layout;
    void *vehicle = at<void *>(entity, car);
    if (!vehicle)
        return 1.0L;
    void *glue_tuning = bytes(system(race_tuning_system)) + tuning;
    for (int index = 0; index < at<int>(entity, neighbor_count); ++index) {
        void *other = invoke<void *>(at<void *>(entity, neighbors + Dword(index) * sizeof(Dword)),
                                     player_view_method);
        if (!other)
            continue;
        const int racers = invoke<int>(at<void *>(system(race_rules_system), race_rules_controller),
                                        racer_count_method);
        if (racers < 2)
            return 1.0L;

        const Dword own_place = at<Dword>(entity, race_position) + 1;
        const Dword other_place = at<Dword>(other, race_position) + 1;
        const long double speed = invoke<long double>(other, speed_method);
        float other_speed = 0.0L < speed ? float(speed) : 0.0f;
        if (std::int32_t(own_place) < 1 || std::int32_t(own_place) > racers)
            return 1.0L;

        const long double rank_difference = signed_integer(other_place - own_place);
        float rank_fraction;
        if (0.0L < rank_difference)
            rank_fraction = float(rank_difference / (signed_integer(other_place - 1) + 1.0f));
        else if (rank_difference < 0.0L)
            rank_fraction = float((-1.0L / (signed_integer(Dword(racers) - other_place) + 1.0f)) *
                                  rank_difference);
        else
            rank_fraction = 0.0f;

        const float *other_position = invoke<const float *>(other, 0);
        const float *position = invoke<const float *>(entity, 0);
        const float distance = float(racer_distance(position, other_position));
        const int completed = participant_completed_checkpoints(
            system(race_rules_system), at<Dword>(vehicle, participant));
        const int laps = at<int>(system(race_settings_system), lap_count);
        float lap_factor = 1.0f;
        if (laps > 1 && completed > 0) {
            const long double remaining_fraction = 1.0L - 1.0L / static_cast<long double>(laps);
            const long double closed_laps = remaining_fraction * at<float>(glue_tuning, close_lap_factor) *
                                            static_cast<long double>(completed);
            lap_factor = float(1.0L - closed_laps / signed_integer(Dword(laps) - 1));
        }

        float gain;
        long double result;
        if (std::int32_t(own_place) > std::int32_t(other_place)) {
            gain = float(absolute_value(number(at<float>(glue_tuning, speedup))));
            other_speed = float(absolute_value(number(other_speed) * at<float>(glue_tuning, speedup_slack)) *
                                lap_factor);
            if (distance < other_speed)
                gain = apply_glue_falloff(distance, other_speed,
                                         &at<float>(glue_tuning, falloff_behind), gain);
            void *body = invoke<void *>(entities::interface_at(entity, body_interface), 0);
            result = body && at<Byte>(body, body_active)
                         ? (1.0L - number(lap_factor) * at<float>(glue_tuning, spread_behind) * rank_fraction) * gain
                         : number(gain);
        } else {
            gain = float(-absolute_value(number(at<float>(glue_tuning, slowdown))));
            other_speed = float(absolute_value(number(other_speed) * at<float>(glue_tuning, slowdown_slack)));
            if (distance < other_speed)
                gain = apply_glue_falloff(distance, other_speed,
                                         &at<float>(glue_tuning, falloff_ahead), gain);
            void *body = invoke<void *>(entities::interface_at(entity, body_interface), 0);
            result = body && at<Byte>(body, body_active)
                         ? (1.0L - number(rank_fraction) * at<float>(glue_tuning, spread_ahead)) * gain
                         : number(gain);
        }

        if (other_place == 1)
            return (number(at<float>(glue_tuning, winner_boost)) + 1.0f) * result + 1.0f;
        if (std::int32_t(other_place) >= racers)
            result = (number(at<float>(glue_tuning, last_place_boost)) + 1.0f) * result;
        return result + 1.0f;
    }
    return 1.0L;
}
} // namespace nfs::game::runtime::ai::cars
