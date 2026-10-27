#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
namespace nfs::game::runtime::ai::lifecycle::pending {
namespace {
struct RaceTuningEntry {
    unsigned section;
    const char *key;
    unsigned offset;
    bool integer;
};

const RaceTuningEntry race_tuning_entries[] = {
    {0, "Player.ResetSpeedMPH", 84420, false},
    {1, "Player.RoadblockWaitTime", 84424, false},
    {0, "Racer.Glue.slowdown", 84428, false},
    {0, "Racer.Glue.slowdown_slack", 84432, false},
    {0, "Racer.Glue.spread_ahead", 84436, false},
    {0, "Racer.Glue.speedup", 84440, false},
    {0, "Racer.Glue.speedup_slack", 84444, false},
    {0, "Racer.Glue.spread_behind", 84448, false},
    {0, "Racer.Glue.close_lap_factor", 84452, false},
    {0, "Racer.Glue.winnerboost", 84464, false},
    {0, "Racer.Glue.looserboost", 84468, false},
    {0, "Racer.Glue.falloff_ahead", 84456, false},
    {0, "Racer.Glue.falloff_behind", 84460, false},
    {0, "Racer.OverallSpeedFactor", 84492, false},
    {0, "Racer.SkillSpeedRange", 84488, false},
    {0, "Racer.ClassFactor", 84496, false},
    {0, "Racer.MaxPathDifficulty", 84500, true},
    {0, "Racer.MinPathDifficulty", 84504, true},
    {0, "Racer.ManualResetTime", 84508, false},
    {0, "Racer.ResetSpeedMPH", 84512, false},
    {0, "Racer.FastResetDist", 84516, false},
    {1, "Cop.warpFrom", 84520, false},
    {1, "Cop.warpTo", 84524, false},
    {1, "Cop.contentLagTime", 84528, false},
    {1, "Cop.contentLagDist", 84532, false},
    {1, "Cop.contentLagDistMax", 84536, false},
    {1, "Cop.stunModulus", 84540, true},
    {1, "Cop.Threat.radius", 84544, false},
    {1, "Cop.Threat.racePositionFactor", 84548, false},
    {1, "Cop.Bust.radius", 84552, false},
    {1, "Cop.Bust.speed", 84556, false},
    {1, "Cop.Bust.time", 84560, false},
    {1, "Cop.Radar.time", 84564, false},
    {1, "Perp.warpPerps", 84568, true},
    {1, "Perp.warpFrom", 84572, false},
    {1, "Perp.warpTo", 84576, false},
    {1, "Perp.firstWarpTo", 84580, false},
    {1, "Perp.defaultGlue", 84584, false},
    {1, "Perp.glueDist", 84588, false},
    {1, "Perp.glueDeadZone", 84592, false},
    {1, "Perp.minDamageImpact", 84596, false},
    {1, "Perp.maxDamageImpact", 84600, false},
    {1, "Perp.gracePeriod", 84604, false},
    {1, "Perp.projectileDamage", 84608, false},
    {1, "Copter.minimumPayload", 84628, true},
    {2, "Racer.ObserveCar.oncomming", 84472, false},
    {2, "Racer.ObserveCar.following", 84476, false},
    {2, "Racer.ObserveCar.side", 84480, false},
    {2, "Racer.ObserveCar.blocking", 84484, false},
    {2, "Traffic.ObserveCar.oncomming", 84612, false},
    {2, "Traffic.ObserveCar.following", 84616, false},
    {2, "Traffic.ObserveCar.side", 84620, false},
    {2, "Traffic.ObserveCar.blocking", 84624, false},
    {2, "Global.ObserveCar.oncomming", 84404, false},
    {2, "Global.ObserveCar.following", 84408, false},
    {2, "Global.ObserveCar.side", 84412, false},
    {2, "Global.ObserveCar.blocking", 84416, false},
    {2, "Global.ObserveWorld.best", 84396, false},
    {2, "Global.ObserveWorld.driveside", 84400, false},
    {2, "Global.ObserveWorld.walls", 84392, false},
    {1, "Roadblock.playerDistance", 84632, false},
};
} // namespace

void read_race_tuning(Dword *owner) {
    level_file("aipaths.dat");
    char filename[260];
    std::sprintf(filename, "%sracetune.ini", reinterpret_cast<const char *>(system(4)) + 3644);
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        const int difficulty = at<int>(system(72), 192);
        const char *normal, *pursuit;
        if (difficulty == 0) {
            normal = "Beginner";
            pursuit = "Beginner_CopMode";
        } else if (difficulty == 1) {
            normal = "Intermediate";
            pursuit = "Intermediate_CopMode";
        } else if (difficulty == 2) {
            normal = "Advanced";
            pursuit = "Advanced_CopMode";
        }
        // The saved method has no default section for an invalid difficulty.
        const char *sections[] = {at<Dword>(system(72), 172) ? pursuit : normal, pursuit,
                                  "Observations"};
        for (const auto &entry : race_tuning_entries) {
            if (entry.integer)
                ini::get_int(&reader, sections[entry.section], entry.key,
                             &at<int>(owner, entry.offset));
            else
                ini::get_float(&reader, sections[entry.section], entry.key,
                               &at<float>(owner, entry.offset));
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
