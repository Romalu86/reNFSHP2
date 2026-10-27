#include "../../police/pursuit/police_chase_x87.hpp"
#include "ai_chase_tuning.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cmath>
namespace nfs::game::runtime::ai::lifecycle::pending {
// Level0 does not write the table.
void set_pursuit_level_duration(Dword level, Dword seconds) {
    if (level && level < 6)
        pursuit_level_seconds[level - 1] = seconds;
}

void read_police_chase_tuning() {
    char filename[260];
    std::sprintf(filename, "%sAIChaseDefs.ini", reinterpret_cast<const char *>(system(4)) + 3644);
    if (!files::file::exists(filename))
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        float parsed = 0.0f;
        int integer = 0;
        auto ticks = [](float seconds) { return police_frame::chase_ticks(seconds); };
        if (ini::get_float(&reader, "chase", "copSecondsBetweenTacticCalls", &parsed))
            state<Dword>(7352944) = ticks(parsed);
        float evade_percentage = 50.0f;
        const int difficulty = at<int>(system(72), 192);
        if (difficulty == 0) {
            ini::get_bool(&reader, "chase", "easyCopMeterThreshold", &cop_meter_threshold);
            ini::get_float(&reader, "chase", "easyPerpEvadeRoadblock", &evade_percentage);
        } else if (difficulty == 1) {
            ini::get_bool(&reader, "chase", "mediumCopMeterThreshold", &cop_meter_threshold);
            ini::get_float(&reader, "chase", "mediumPerpEvadeRoadblock", &evade_percentage);
        } else if (difficulty == 2) {
            ini::get_bool(&reader, "chase", "hardCopMeterThreshold", &cop_meter_threshold);
            ini::get_float(&reader, "chase", "hardPerpEvadeRoadblock", &evade_percentage);
        }
        roadblock_evade_fraction =
            police_frame::evade_fraction(evade_percentage);
        ini::get_float(&reader, "chase", "perpMinChaseSpeed", &minimum_chase_speed);
        if (ini::get_int(&reader, "chase", "initialSpawnDelay", &integer))
            state<Dword>(7352464) = Dword(integer) * 60;
        const char *duration_keys[] = {"pursuitLevel_0_Duration", "pursuitLevel_1_Duration",
                                       "pursuitLevel_2_Duration", "pursuitLevel_3_Duration",
                                       "pursuitLevel_4_Duration", "pursuitLevel_5_Duration"};
        for (Dword level = 0; level < 6; ++level)
            if (ini::get_int(&reader, "chase", duration_keys[level], &integer))
                set_pursuit_level_duration(level, Dword(integer));
        ini::get_int(&reader, "roadblock", "startPursuitLevel",
                     reinterpret_cast<int *>(&roadblock_start_level));
        ini::get_int(&reader, "roadblock", "startPursuitStrategy",
                     reinterpret_cast<int *>(&roadblock_start_strategy));
        ini::get_int(&reader, "roadblock", "freezeCops", &state<int>(7352732));
        ini::get_float(&reader, "roadblock", "spikestripSize", &spike_strip_size);
        ini::get_float(&reader, "roadblock", "fudgeSpacing", &roadblock_spacing);
        if (ini::get_int(&reader, "roadblock", "secondsMinBackupWait", &state<int>(7352472)))
            state<Dword>(7352472) *= 60;
        if (ini::get_int(&reader, "roadblock", "secondsMaxBackupWait", &state<int>(7352468)))
            state<Dword>(7352468) *= 60;
        float cop_distance = 0.0f, spike_distance = 0.0f;
        ini::get_float(&reader, "chase", "playerBustResetCopDist", &cop_distance);
        if (cop_distance != 0.0f && !std::isnan(cop_distance))
            player_reset_cop_distance = cop_distance;
        ini::get_float(&reader, "chase", "playerBustResetSpikeDist", &spike_distance);
        if (spike_distance != 0.0f && !std::isnan(spike_distance))
            player_reset_spike_distance = spike_distance;
        if (ini::get_int(&reader, "chase", "disablePerpPenalties", &integer))
            state<Dword>(7352736) = integer != 0;
        if (ini::get_int(&reader, "chase", "copBoredomThreshold", &integer))
            state<Dword>(7352420) = Dword(integer) * 60;
        if (ini::get_float(&reader, "chase", "copSpawnDistanceThreshold", &parsed))
            spawn_distance = parsed;
        if (ini::get_int(&reader, "chase", "SingleRaceSupportMax", &integer))
            at<int>(system(72), 2308) = integer;
        if (ini::get_int(&reader, "chase", "SecondsImmunityBeforeFinish", &integer))
            finish_immunity_seconds = Dword(integer);
        if (ini::get_float(&reader, "helicopter", "helicopterActivationDelay", &parsed))
            helicopter_activation_ticks = ticks(parsed);
        if (ini::get_int(&reader, "helicopter", "helicopterMaxFlightTime", &integer))
            state<Dword>(7352836) = Dword(integer) * 60;
        if (ini::get_float(&reader, "helicopter", "helicopterBladeRotationIncrement", &parsed))
            helicopter_rotor_increment = parsed;
        if (ini::get_float(&reader, "helicopter", "helicopterProjectileDropHeight", &parsed))
            projectile_drop_height = parsed;
        if (ini::get_float(&reader, "helicopter", "helicopterProjectileDropBack", &parsed))
            projectile_drop_back = parsed;
        if (ini::get_float(&reader, "helicopter", "helicopterProjectileDropFreqMin", &parsed))
            state<Dword>(7352832) = ticks(parsed);
        if (ini::get_float(&reader, "helicopter", "helicopterProjectileDropFreqMax", &parsed))
            state<Dword>(7352828) = ticks(parsed);
        if (ini::get_float(&reader, "helicopter", "helicopterPatternWaitMin", &parsed))
            state<Dword>(7352824) = ticks(parsed);
        if (ini::get_float(&reader, "helicopter", "helicopterPatternWaitMax", &parsed))
            state<Dword>(7352820) = ticks(parsed);
        if (ini::get_float(&reader, "helicopter", "helicopterProjectileMass", &parsed))
            projectile_mass = parsed;
        if (ini::get_float(&reader, "helicopter", "helicopterProjectileElasticity", &parsed))
            projectile_elasticity = parsed;
        if (ini::get_float(&reader, "helicopter", "helicopterProjectileLifetime", &parsed))
            state<Dword>(7352864) = ticks(parsed);
        if (ini::get_int(&reader, "cop stun", "minAICopStunPeriod", &integer))
            minimum_cop_stun_ticks = integer ? Dword(integer) * 60 : 1;
        if (ini::get_int(&reader, "cop stun", "maxAICopStunPeriod", &integer))
            state<Dword>(7352432) = Dword(integer) * 60;
        if (ini::get_int(&reader, "cop stun", "copCanBeStunned", &integer))
            cop_can_be_stunned = integer != 0;
        if (ini::get_int(&reader, "cop stun", "copCanBeStunnedByBoomObject", &integer))
            state<Dword>(7352436) = integer != 0;
        if (ini::get_float(&reader, "cop stun", "copStunThreshold", &parsed))
            state<float>(7352440) = parsed;
        if (ini::get_int(&reader, "cop stun", "copCanBeStunnedByAITraffic", &integer))
            traffic_can_stun_cop = integer != 0;
        if (ini::get_int(&reader, "cop stun", "copCanBeStunnedByAIRacers", &integer))
            racer_can_stun_cop = integer != 0;
        if (ini::get_float(&reader, "cop stun", "copStunRadius", &parsed))
            cop_stun_radius = parsed;
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
