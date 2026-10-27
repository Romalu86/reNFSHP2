#include "character_runtime.hpp"
#include <cstdio>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::world::actor_resources {

void load_actor_sound_catalog() {
    using audio::state::track;
    track<Dword>(audio_layout::actor_sound_count) = track<Dword>(audio_layout::actor_sound_event_count) = track<Dword>(11892) = 0;
    char filename[260];
    // All four saved race-direction/weather branches select this same file.
    std::strcpy(filename, runtime::track_file("traudio.ini"));
    if (!files::file::exists(filename)) {
        track<Dword>(audio_layout::actor_sound_count) = track<Dword>(audio_layout::actor_sound_event_count) = track<Dword>(11892) = 0;
        return;
    }
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        for (int index = 0; index < 20; ++index) {
            char section[44], type[256] = {};
            std::sprintf(section, "anim%d", index);
            if (!ini::get_string(&reader, section, "type", type, 255))
                break;
            const unsigned entry_offset = audio_layout::actor_sounds + unsigned(index) * 36;
            std::sscanf(type, "%d", &track<int>(entry_offset));
            ini::get_int(&reader, section, "patch", &track<int>(entry_offset + 4));
            ini::get_int(&reader, section, "maxvol", &track<int>(entry_offset + 12));
            ini::get_int(&reader, section, "minvol", &track<int>(entry_offset + 16));
            ini::get_int(&reader, section, "dist", &track<int>(entry_offset + 8));
            ini::get_int(&reader, section, "oneshot", &track<int>(entry_offset + 20));
            ini::get_int(&reader, section, "nomovesilent", &track<int>(entry_offset + 24));
            ini::get_int(&reader, section, "fullvol", &track<int>(entry_offset + 32));
            ++track<Dword>(audio_layout::actor_sound_count);
        }
        for (int index = 0; index < 20; ++index) {
            char section[44], patch[256] = {};
            std::sprintf(section, "animevent%d", index);
            if (!ini::get_string(&reader, section, "patch", patch, 255))
                break;
            const unsigned entry_offset = audio_layout::actor_sound_events + unsigned(index) * 36;
            std::sscanf(patch, "%d", &track<int>(entry_offset));
            ini::get_int(&reader, section, "maxvol", &track<int>(entry_offset + 8));
            ini::get_int(&reader, section, "minvol", &track<int>(entry_offset + 12));
            ini::get_int(&reader, section, "dist", &track<int>(entry_offset + 4));
            ini::get_int(&reader, section, "oneshot", &track<int>(entry_offset + 16));
            ini::get_int(&reader, section, "fullvol", &track<int>(entry_offset + 28));
            ++track<Dword>(audio_layout::actor_sound_event_count);
        }
        ini::reader_clear(&reader);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    track<Dword>(11892) = 0;
}

int find_actor_sound(int identifier) {
    using audio::state::track;
    if (identifier)
        for (int index = 0; index < track<int>(audio_layout::actor_sound_count); ++index)
            if (track<int>(audio_layout::actor_sounds + unsigned(index) * 36) == identifier)
                return index;
    return -1;
}
} // namespace nfs::game::world::actor_resources
