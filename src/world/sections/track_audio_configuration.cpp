#include "nfs/audio/music/music_clients.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include <cstdio>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;
struct PointSound {
    int event, random, oneshot, density, distance, maximum, minimum;
    Dword serial;
    float position[3];
    int play_code, lap;
    Dword reserved[3];
};
struct EmitterInfo {
    char name[40];
    Dword flags;
    int patch, variation, distance, maximum, minimum;
    Dword reserved;
    int full_volume;
};
struct MusicTrigger {
    int intensity, event;
    float first[3], second[3];
    Dword reserved;
    int trigger;
};
struct TriggerEvent {
    int type, parameters[6];
    float values[2];
};
static_assert(sizeof(PointSound) == 64 && sizeof(EmitterInfo) == 72 && sizeof(MusicTrigger) == 40 &&
              sizeof(TriggerEvent) == 36);

void read_sound_points() {
    track<Dword>(audio_layout::sound_point_count) = track<Dword>(11892) = 0;
    char filename[260];
    std::strcpy(filename, runtime::track_file("audiopts.ini"));
    if (!files::file::exists(filename)) {
        track<Dword>(audio_layout::sound_point_count) = track<Dword>(11892) = 0;
        return;
    }
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        unsigned index = 0;
        for (unsigned offset = audio_layout::sound_points; offset < audio_layout::music_triggers; offset += 64, ++index) {
            PointSound &point = track<PointSound>(offset);
            char section[44], event[256]{};
            std::sprintf(section, "soundpoint%d", index);
            if (!ini::get_string(&reader, section, "SFXnum", event, 255))
                break;
            std::sscanf(event, "%d", &point.event);
            ini::get_int(&reader, section, "SFXrand", &point.random);
            ini::get_int(&reader, section, "oneshot", &point.oneshot);
            ini::get_int(&reader, section, "density", &point.density);
            ini::get_int(&reader, section, "dist", &point.distance);
            ini::get_float(&reader, section, "Xpos", &point.position[0]);
            ini::get_float(&reader, section, "Ypos", &point.position[1]);
            ini::get_float(&reader, section, "Zpos", &point.position[2]);
            ini::get_int(&reader, section, "playcode", &point.play_code);
            ini::get_int(&reader, section, "lapnum", &point.lap);
            ini::get_int(&reader, section, "maxvol", &point.maximum);
            ini::get_int(&reader, section, "minvol", &point.minimum);
            point.serial = runtime::race_audio::next_emitter_serial();
            ++track<Dword>(audio_layout::sound_point_count);
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
    track<Dword>(11892) = 0;
}

void read_emitter_info() {
    track<Dword>(audio_layout::emitter_count) = track<Dword>(11892) = 0;
    char filename[260];
    std::strcpy(filename, runtime::track_file("traudio.ini"));
    if (!files::file::exists(filename)) {
        track<Dword>(audio_layout::emitter_count) = track<Dword>(11892) = 0;
        return;
    }
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        unsigned index = 0;
        for (unsigned offset = audio_layout::emitter_info; offset + 40 < 11860; offset += 72, ++index) {
            EmitterInfo &info = track<EmitterInfo>(offset);
            char section[128];
            std::sprintf(section, "emitterinfo%d", index);
            if (!ini::get_string(&reader, section, "name", info.name, 40))
                break;
            int type;
            ini::get_int(&reader, section, "type", &type);
            info.flags = type == 0 ? 520 : type == 1 ? 264 : type == 2 ? 516 : Dword(-1);
            ini::get_int(&reader, section, "patch", &info.patch);
            ini::get_int(&reader, section, "patchrand", &info.variation);
            ini::get_int(&reader, section, "audibledist", &info.distance);
            ini::get_int(&reader, section, "maxvol", &info.maximum);
            ini::get_int(&reader, section, "minvol", &info.minimum);
            ini::get_int(&reader, section, "fullvol", &info.full_volume);
            ++track<Dword>(audio_layout::emitter_count);
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
    track<Dword>(11892) = 0;
}

void read_trigger_events() {
    track<Dword>(audio_layout::trigger_event_count) = track<Dword>(11892) = 0;
    char filename[260];
    std::strcpy(filename, runtime::track_file("audtrigs.ini"));
    if (!files::file::exists(filename)) {
        track<Dword>(audio_layout::trigger_event_count) = track<Dword>(11892) = 0;
        return;
    }
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        unsigned index = 0;
        for (unsigned offset = audio_layout::trigger_events; offset + 4 < 9304; offset += 36, ++index) {
            TriggerEvent &event = track<TriggerEvent>(offset);
            char section[44], type[256]{};
            std::sprintf(section, "triggerevent%d", index);
            if (!ini::get_string(&reader, section, "type", type, 255))
                break;
            std::sscanf(type, "%d", &event.type);
            for (unsigned parameter = 0; parameter < 6; ++parameter) {
                char key[8];
                std::sprintf(key, "intp%d", parameter + 1);
                ini::get_int(&reader, section, key, event.parameters + parameter);
            }
            ini::get_float(&reader, section, "floatp1", event.values);
            ini::get_float(&reader, section, "floatp2", event.values + 1);
            ++track<Dword>(audio_layout::trigger_event_count);
        }
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
    track<Dword>(11892) = 0;
}

void read_music_triggers() {
    track<Dword>(audio_layout::music_trigger_count) = track<Dword>(11892) = 0;
    char filename[260];
    std::strcpy(filename, runtime::track_file("audtrigs.ini"));
    if (!files::file::exists(filename))
        track<Dword>(audio_layout::music_trigger_count) = 0;
    else {
        ini::Reader reader;
        ini::reader_construct(&reader, filename, 0);
        try {
            unsigned index = 0;
            for (unsigned offset = audio_layout::music_triggers; offset + 36 < audio_layout::emitter_info; offset += 40, ++index) {
                MusicTrigger &trigger = track<MusicTrigger>(offset);
                char section[44], intensity[256]{}, key[32];
                std::sprintf(section, "musictrigger%d", index);
                if (!ini::get_string(&reader, section, "intensity", intensity, 255))
                    break;
                std::sscanf(intensity, "%d", &trigger.intensity);
                ini::get_int(&reader, section, "event", &trigger.event);
                std::sprintf(key, "trigevent%d", 0);
                ini::get_int(&reader, section, key, &trigger.trigger);
                const char *first[3] = {"Xpt1", "Ypt1", "Zpt1"}, *second[3] = {"Xpt2", "Ypt2", "Zpt2"};
                for (unsigned axis = 0; axis < 3; ++axis)
                    ini::get_float(&reader, section, first[axis], trigger.first + axis);
                for (unsigned axis = 0; axis < 3; ++axis)
                    ini::get_float(&reader, section, second[axis], trigger.second + axis);
                ++track<Dword>(audio_layout::music_trigger_count);
            }
        } catch (...) {
            ini::reader_clear(&reader);
            throw;
        }
        ini::reader_clear(&reader);
    }
    track<Dword>(11892) = 0;
    read_trigger_events();
}

void load_track_audio_configuration() {
    track<Dword>(11892) = 0;
    char filename[260];
    std::strcpy(filename, runtime::track_file("traudio.ini"));
    if (files::file::exists(filename)) {
        ini::Reader reader;
        ini::reader_construct(&reader, filename, 0);
        try {
            ini::get_int(&reader, "trackvol", "volume", &configuration<int>(audio_layout::track_volume));
            unsigned index = 0;
            for (unsigned offset = audio_layout::wind_sections; offset < audio_layout::sound_point_count; offset += 16, ++index) {
                char section[44];
                std::sprintf(section, "wind%d", index);
                ini::get_int(&reader, section, "start", &track<int>(offset));
                if (track<int>(offset) == -1)
                    break;
                ini::get_int(&reader, section, "end", &track<int>(offset + 4));
                ini::get_int(&reader, section, "SFXnum", &track<int>(offset + 8));
                ini::get_int(&reader, section, "trackside", &track<int>(offset + 12));
            }
        } catch (...) {
            ini::reader_clear(&reader);
            throw;
        }
        ini::reader_clear(&reader);
    }
    read_sound_points();
    read_emitter_info();
    read_music_triggers();
    track<Dword>(11892) = 0;
}

void reset_audio_world() {
    track<Dword>(11820) = Dword(-1);
    for (unsigned offset = 11828; offset < 11884; offset += 28) {
        track<Dword>(offset + 8) = 0;
        track<Dword>(offset) = 0;
    }
    track<Dword>(9256) = 0;
    track<Dword>(9260) = Dword(-1);
    for (unsigned offset = 132; offset < 852; offset += 36)
        track<Byte>(offset) = 0;
    configuration<Dword>(1692) = 64;
    configuration<Dword>(1696) = 127;
    const Dword hardware[8] = {0, 13, 15, 17, 40, 0, 0, 0}, software[8] = {0, 1, 1, 1, 2, 0, 0, 0};
    for (unsigned index = 0; index < 8; ++index) {
        configuration<Dword>(1708 + index * 4) = hardware[index];
        configuration<Dword>(1740 + index * 4) = software[index];
    }
    load_track_audio_configuration();
}
} // namespace nfs::game::audio
