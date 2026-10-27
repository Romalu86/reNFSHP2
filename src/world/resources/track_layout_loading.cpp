#include "track_render_resources.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::track_render {

float glow_distance = 0, glow_fade = 10, glow_scale = 50, glow_distance_secondary = .5f,
      glow_fade_secondary = 6, glow_scale_secondary = 50;
} // namespace nfs::game::world::track_render
namespace nfs::services::pending {
alignas(4) Byte ambient_shadow[16]{};
}
namespace nfs::game::world::pending {

void read_track_layout(void *owner) {
    using namespace track_render;
    char filename[260], section[256];
    std::sprintf(filename, runtime::level_file("lighting.ini"));
    ini::Reader lighting;
    ini::reader_construct(&lighting, filename, 0);
    try {
        const char *glow_keys[] = {"GlowDistScale",  "GlowFadeScale",  "GlowScale",
                                   "GlowDistScale2", "GlowFadeScale2", "GlowScale2"};
        float *glow_values[] = {&glow_distance,       &glow_fade,
                                &glow_scale,          &glow_distance_secondary,
                                &glow_fade_secondary, &glow_scale_secondary};
        for (unsigned index = 0; index < 6; ++index)
            ini::get_float(&lighting, "TrackGlows", glow_keys[index], glow_values[index]);
        const char *shadow_keys[] = {"ShadowColorR", "ShadowColorG", "ShadowColorB",
                                     "ShadowColorA"};
        for (unsigned index = 0; index < 4; ++index)
            ini::get_float(&lighting, "TrackShadow", shadow_keys[index],
                           reinterpret_cast<float *>(services::pending::ambient_shadow) + index);
        at<Dword>(owner, 4) = 0;
        std::sprintf(filename, runtime::level_file("drvpath.ini"));
        ini::Reader path;
        ini::reader_construct(&path, filename, 0);
        try {
            ini::get_int(&path, "path", "nodenum", &at<int>(owner, 4));
            if (at<int>(owner, 4) >= 64)
                at<int>(owner, 4) = 63;
            int tree = -1, event = -1;
            void *rules = subsystem(72);
            if (static_cast<Byte>(services::special_race_mode(rules))) {
                switch (at<int>(rules, 148)) {
                case 2:
                    tree = 0;
                    break;
                case 3:
                    tree = 1;
                    break;
                case 4:
                    tree = 2;
                    break;
                }
                event = at<int>(rules, 102920);
            }
            bool cops = false, racers = false;
            for (Dword index = 0; index < at<Dword>(rules, 168); ++index) {
                void *participant = index < 24 ? offset(rules, 248 + index * 84) : nullptr;
                if (at<Dword>(participant, 0)) {
                    if (at<Dword>(participant, 8))
                        cops = true;
                    else
                        racers = true;
                }
            }
            std::memset(offset(owner, 16), 0, 32);
            if (cops && !racers) {
                for (unsigned player = 0; player < 2; ++player) {
                    std::sprintf(section, "PlayerCop%d", player + 1);
                    ini::get_int(&path, section, "startNode", &at<int>(owner, 16 + player * 4));
                    constexpr const char *keys[] = {"posX", "posY", "posZ"};
                    for (unsigned axis = 0; axis < 3; ++axis)
                        ini::get_float(&path, section, keys[axis],
                                       &at<float>(owner, 24 + player * 12 + axis * 4));
                }
            } else {
                std::sprintf(section, "Tree%dEvent%d", tree, event);
                ini::get_int(&path, section, "startNode", &at<int>(owner, 16));
                at<Dword>(owner, 20) = at<Dword>(owner, 16);
            }
            for (int index = 0; index < at<int>(owner, 4); ++index) {
                void *pair = offset(owner, Dword(index) * 336);
                std::sprintf(section, "%s%d", "node", index);
                construct_compartment(offset(pair, 168), Dword(index));
                ini::get_int(&path, section, "compartmentId", &at<int>(pair, 168));
                construct_compartment(offset(pair, 336), Dword(index));
                ini::get_int(&path, section, "compartmentId", &at<int>(pair, 336));
                at<Dword>(pair, 172) = at<Dword>(pair, 340) = 1032805417;
                at<Dword>(pair, 176) = at<Dword>(pair, 344) = 998445679;
                at<Dword>(pair, 200) = 1165623296;
                at<Dword>(pair, 196) = 0;
                at<Dword>(pair, 204) = 1120403456;
                at<Dword>(pair, 208) = 4;
                at<Byte>(pair, 212) = 1;
                at<Dword>(pair, 216) = 0;
                at<Dword>(pair, 220) = 90;
                int red = 133, green = 131, blue = 123, range_adjust = 1, base_equals_sun = 1;
                std::sprintf(section, "%s%d", "effects", index);
                ini::get_int(&path, section, "BaseEqSun", &base_equals_sun);
                at<Byte>(pair, 224) = base_equals_sun == 1;
                ini::get_float(&path, section, "sunFogDensity", &at<float>(pair, 172));
                ini::get_float(&path, section, "sunFogSkyDensity", &at<float>(pair, 176));
                ini::get_int(&path, section, "sunFogColorR", &red);
                ini::get_int(&path, section, "sunFogColorG", &green);
                ini::get_int(&path, section, "sunFogColorB", &blue);
                auto packed_colour = [&] {
                    return (((Dword(red) | 4294967040u) << 8 | (Dword(green) & 255)) << 8) |
                           (Dword(blue) & 255);
                };
                at<Dword>(pair, 192) = packed_colour();
                if (!at<Byte>(pair, 224)) {
                    ini::get_float(&path, section, "baseFogDensity", &at<float>(pair, 340));
                    ini::get_float(&path, section, "baseFogSkyDensity", &at<float>(pair, 344));
                    ini::get_int(&path, section, "baseFogColorR", &red);
                    ini::get_int(&path, section, "baseFogColorG", &green);
                    ini::get_int(&path, section, "baseFogColorB", &blue);
                    at<Dword>(pair, 360) = packed_colour();
                } else {
                    at<Dword>(pair, 340) = at<Dword>(pair, 172);
                    at<Dword>(pair, 344) = at<Dword>(pair, 176);
                    at<Dword>(pair, 360) = at<Dword>(pair, 192);
                }
                ini::get_float(&path, section, "ngcFogFarClip", &at<float>(pair, 200));
                ini::get_float(&path, section, "ngcFogNearClip", &at<float>(pair, 196));
                ini::get_float(&path, section, "ngcFogSkyFog", &at<float>(pair, 204));
                ini::get_int(&path, section, "ngcFogType", &at<int>(pair, 208));
                ini::get_int(&path, section, "ngcFogRangeAdjust", &range_adjust);
                ini::get_int(&path, section, "ngcFogRampFramespan", &at<int>(pair, 220));
                at<Byte>(pair, 212) = range_adjust != 0;
                ini::get_int(&path, section, "ngcFogColorR", &red);
                ini::get_int(&path, section, "ngcFogColorG", &green);
                ini::get_int(&path, section, "ngcFogColorB", &blue);
                at<Dword>(pair, 216) = packed_colour();
                at<Dword>(pair, 180) = at<Dword>(pair, 184) = 0;
                at<Dword>(pair, 188) = at<Dword>(pair, 356) = 2;
                at<Dword>(pair, 348) = at<Dword>(pair, 352) = 0;
            }
            dependency::reset_track_directory(offset(owner, 76));
        } catch (...) {
            ini::reader_clear(&path);
            throw;
        }
        ini::reader_clear(&path);
    } catch (...) {
        ini::reader_clear(&lighting);
        throw;
    }
    ini::reader_clear(&lighting);
}
} // namespace nfs::game::world::pending
