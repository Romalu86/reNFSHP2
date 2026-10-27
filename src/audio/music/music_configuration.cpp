#include "nfs/audio/music/music_clients.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include <cstring>
#include <cstdio>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {
using namespace state;

void read_volumes() {
    char filename[260];
    std::sprintf(filename, "%svolumes.ini", path(1304));
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    const char *keys[12] = {"player",   "player",   "carcol",      "boomcol",
                            "music",    "boomloop", "boomoneshot", "speech",
                            "opponent", "track",    "playerskid",  "traffic"};
    const unsigned volume_offsets[12] = {audio_layout::player_volume, audio_layout::second_player_volume, audio_layout::car_collision_volume, audio_layout::boom_collision_volume, audio_layout::music_volume, audio_layout::boom_loop_volume,
                                    audio_layout::boom_oneshot_volume, audio_layout::speech_volume, audio_layout::opponent_volume, audio_layout::track_volume, audio_layout::player_skid_volume, audio_layout::traffic_volume};
    for (unsigned key = 0; key < 12; ++key)
        ini::get_int(&reader, "volume", keys[key], &configuration<int>(volume_offsets[key]));
    const unsigned clamp_order[12] = {audio_layout::boom_collision_volume, audio_layout::boom_loop_volume, audio_layout::boom_oneshot_volume, audio_layout::car_collision_volume, audio_layout::speech_volume, audio_layout::music_volume,
                                      audio_layout::opponent_volume, audio_layout::player_volume, audio_layout::second_player_volume, audio_layout::player_skid_volume, audio_layout::track_volume, audio_layout::traffic_volume};
    for (unsigned offset : clamp_order) {
        int &volume = configuration<int>(offset), maximum = offset == audio_layout::music_volume ? 100 : 127;
        if (volume < 0)
            volume = 0;
        else if (volume > maximum)
            volume = maximum;
    }
    for (unsigned offset : volume_offsets)
        configuration<int>(offset + 52) = configuration<int>(offset);
    ini::reader_clear(&reader);
}

void initialize_configuration() {
    configuration<Dword>(audio_layout::audio_device_enabled) = configuration<Dword>(audio_layout::audio_enabled) = 1;
    const int defaults[13] = {50, 50, 75, 127, 75, 75, 110, 127, 0, 70, 127, 50, 127};
    for (unsigned index = 0; index < 13; ++index) {
        configuration<int>(audio_layout::player_volume + index * 4) = defaults[index];
        configuration<int>(audio_layout::baseline_player_volume + index * 4) = defaults[index];
    }
    configuration<Dword>(8) = configuration<Dword>(12) = configuration<Dword>(audio_layout::music_muted) =
        0;
    library().fade_setting = 8;
    read_volumes();
    apply_event_volumes();
    option<int>(audio_layout::engine_type) = 2;
    option<int>(audio_layout::max_cars) = 0;
    auto *cars = &configuration<CarSound>(audio_layout::car_sounds);
    for (unsigned index = 0; index < 25; ++index) {
        cars[index].bend_range = cars[index].bass_add = cars[index].inside = cars[index].outside =
            0;
        std::strcpy(cars[index].bank, "bad");
        std::strcpy(&option<char>(audio_layout::car_name), "bad");
        std::strcpy(cars[index].engine, "bad");
        std::strcpy(cars[index].exhaust, "bad");
    }
    auto *horns = reinterpret_cast<char(*)[16]>(&configuration<char>(audio_layout::horn_names));
    for (unsigned index = 0; index < 16; ++index)
        std::strcpy(horns[index], "genhorn");
    char filename[284], section[32];
    std::sprintf(filename, "%scar.ini", path(1304));
    if (files::file::exists(filename)) {
        std::memset(section, 0, sizeof(section));
        ini::Reader reader;
        ini::reader_construct(&reader, filename, 0);
        ini::get_int(&reader, "carinfo", "maxcars", &option<int>(audio_layout::max_cars));
        for (int index = 0; index < option<int>(audio_layout::max_cars) && index < 25; ++index) {
            std::sprintf(section, "car%d", index);
            ini::get_string(&reader, section, "vivname", cars[index].bank, 15);
            ini::get_int(&reader, section, "bendrange", &cars[index].bend_range);
            ini::get_int(&reader, section, "bassadd", &cars[index].bass_add);
            ini::get_int(&reader, section, "incar", &cars[index].inside);
            ini::get_int(&reader, section, "outcar", &cars[index].outside);
            ini::get_int(&reader, section, "volume", &cars[index].volume);
            ini::get_string(&reader, section, "name", &option<char>(audio_layout::car_name), 31);
            ini::get_string(&reader, section, "engcode", cars[index].engine, 7);
            ini::get_string(&reader, section, "exhcode", cars[index].exhaust, 7);
        }
        ini::reader_clear(&reader);
    }
    std::sprintf(filename, "%shornz.ini", path(1304));
    if (files::file::exists(filename)) {
        std::memset(section, 0, sizeof(section));
        ini::Reader outer;
        ini::reader_construct(&outer, filename, 0);
        for (unsigned index = 0; index < 16; ++index) {
            ini::Reader reader;
            ini::reader_construct(&reader, filename, 0);
            std::sprintf(section, "horn%d", index);
            ini::get_string(&reader, section, "name", horns[index], 15);
            ini::reader_clear(&reader);
        }
        ini::reader_clear(&outer);
    }
    std::sprintf(filename, "%smusic.ini", path(1044));
    library().instrumental_count = 15;
    library().vocal_count = 8;
    library().first_instrumental = 8;
    std::sprintf(filename, "%splaylist.ini", graphics::startup::configuration_path());
    ini::Reader playlists;
    ini::reader_construct(&playlists, filename, 0);
    ini::get_int(&playlists, "playlists", "number", &option<int>(audio_layout::playlist_count));
    library().custom_playlist = 0;
    read_catalog(0);
    option<Dword>(audio_layout::music_ducked) = 0;
    library().playing = 0;
    library().selection = -1;
    library().reserved_clock = 0;
    sound::field<Byte>(&library().notify, 0) = 0;
    ini::reader_clear(&playlists);
}
} // namespace nfs::game::audio
