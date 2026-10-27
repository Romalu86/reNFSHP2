#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::dependency {
// The pushed argument at callers is ignored by the RET body.
void reset_track_audio_cursor() {
    audio::state::track<Dword>(11892) = 0;
}
namespace pending {

void load_track_object_audio() {
    reset_track_audio_cursor();
    char filename[260], section[260], name[260], material[260];
    std::sprintf(filename, "%sboomobj.ini", offset(subsystem(4), 1304));
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        unsigned next_definition = 10;
        for (unsigned index = 0; index < 20; ++index) {
            std::strcpy(name, &runtime::particles::lifecycle::state<char>(7217172));
            std::sprintf(section, "boominfo%d", index);
            ini::get_string(&reader, section, "name", name, 259);
            bool matched = false;
            constexpr const char *trigger_keys[] = {"trigpatch",  "trigrand", "trigdist",
                                                    "trigmaxvol", "sfxpatch", "sfxrand",
                                                    "sfxdist",    "sfxmaxvol"};
            for (unsigned existing = 0; existing < 10; ++existing) {
                Byte *record = services::pending::boom_sound_definitions + existing * 312;
                if (!std::strcmp(reinterpret_cast<char *>(record), name)) {
                    for (unsigned field = 0; field < 8; ++field)
                        ini::get_int(&reader, section, trigger_keys[field],
                                     &at<int>(record, 280 + field * 4));
                    matched = true;
                }
            }
            if (matched)
                continue;
            ini::get_string(&reader, section, "setmaterial", material, 259);
            Byte *record = services::pending::boom_sound_definitions + next_definition * 312;
            for (unsigned existing = 0; existing < 10; ++existing) {
                Byte *source = services::pending::boom_sound_definitions + existing * 312;
                if (!std::strcmp(reinterpret_cast<char *>(source), material)) {
                    at<Dword>(record, 260) = at<Dword>(source, 260);
                    break;
                }
                at<Dword>(record, 260) = index + 200;
            }
            audio::state::track<Dword>(12200 + index * 4) = next_definition;
            std::strcpy(reinterpret_cast<char *>(record), name);
            at<Dword>(record, 280) = at<Dword>(record, 264) = at<Dword>(record, 296) = Dword(-1);
            const char *keys[] = {"trigpatch", "trigrand", "trigdist", "trigmaxvol",
                                  "colpatch",  "colrand",  "coldist",  "colmaxvol",
                                  "sfxpatch",  "sfxrand",  "sfxdist",  "sfxmaxvol"};
            constexpr unsigned fields[] = {280, 284, 288, 292, 264, 268,
                                           272, 276, 296, 300, 304, 308};
            for (unsigned field = 0; field < 12; ++field)
                ini::get_int(&reader, section, keys[field], &at<int>(record, fields[field]));
            ++next_definition;
        }
        reset_track_audio_cursor();
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace pending
} // namespace nfs::game::world::dependency
