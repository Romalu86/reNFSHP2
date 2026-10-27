#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include <cstring>
#include <cstdio>

namespace nfs::services {

void read_surface_feedback(void *object) {
    const char *keys[] = {
        "kNODRIVE_SURFACE",    "kPAVED_SURFACE",        "kGRAVEL_SURFACE",
        "kGRASS_SURFACE",      "kLEAVES_SURFACE",       "kDIRT_SURFACE",
        "kWATER_SURFACE",      "kWOOD_SURFACE",         "kICE_SURFACE",
        "kSNOW_SURFACE",       "kSurface_Cobble",       "kSurface_Stairs",
        "kSurface_Slabs",      "kSurface_RRTrack",      "kSurface_WetPaved",
        "kSurface_Garbage",    "kSurface_MetalGrating", "kSurface_GroovedPavement",
        "kSurface_Concrete",   "kSurface_MetalPlate",   "kSurface_OilSlick",
        "kSurface_CombedDirt", "kSurface_Tar"};
    auto *reader = static_cast<ini::Reader *>(object);
    for (unsigned index = 0; index < 23; ++index)
        ini::get_int(reader, "Tires_Burnout_Surface_Feedback", keys[index],
                     reinterpret_cast<int *>(pending::surface_feedback + index * 48));
    for (unsigned index = 0; index < 23; ++index)
        ini::get_int(reader, "Tires_Normal_Surface_Feedback", keys[index],
                     reinterpret_cast<int *>(pending::surface_feedback + index * 48 + 4));
}

void load_rumble_configuration() {
    char filename[260];
    const char *paths = reinterpret_cast<char *>(lifecycle::manager->slots[1]);
    std::sprintf(filename, "%sRumbleValues.ini", paths + 524);
    if (!files::file::exists(filename))
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        read_surface_feedback(&reader);
        auto *settings = pending::rumble_values;
        ini::get_float(&reader, "TireRumbleOnSpeed", "fRumbleStartSpeed", settings);
        ini::get_float(&reader, "TireRumbleOnSpeed", "fSpeedFactor", settings + 1);
        ini::get_float(&reader, "TireRumbleOnSpeed", "fSpeedFactorSkid", settings + 2);
        ini::get_int(&reader, "TireRumbleOnSpeed", "nFeedback_TireBlown",
                     reinterpret_cast<int *>(settings + 3));
        ini::get_int(&reader, "TireRumbleOnSpeed", "nFeedback_TireSkids",
                     reinterpret_cast<int *>(settings + 4));
        const char *keys[] = {"nFeedback_CarHitsCar_Hard",  "nFeedback_CarHitsCar_Light",
                              "nFeedback_CarRampCollision", "nFeedback_CarLightCollision",
                              "nFeedback_BottomOutHeavy",   "nFeedback_BottomOutMedium",
                              "nFeedback_BottomOutLight"};
        for (unsigned index = 0; index < 7; ++index)
            ini::get_int(&reader, "Collision_Feedback", keys[index],
                         reinterpret_cast<int *>(settings + 5 + index));
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}


void initialize_input(void *object) {
    load_rumble_configuration();
    pending::configure_input_devices(0, -1);
    reset_input_state(object);
}

void load_input_level(void *object) {
    reset_input_state(object);
    game::runtime::events::subscribe(reinterpret_cast<Dword *>(lifecycle::manager->slots[60 / 4]),
                                     4, reinterpret_cast<Dword *>(pending::input_event));
}
// release-console output is a confirmed RET.
void unload_input_level(void *) {
    game::runtime::events::unsubscribe(
        reinterpret_cast<Dword *>(lifecycle::manager->slots[60 / 4]),
        reinterpret_cast<Dword *>(pending::input_event));
}
} // namespace nfs::services
