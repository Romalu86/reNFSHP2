#include "nfs/audio/music/music_clients.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/resources/files/directory_search.hpp"
#include "nfs/network/session/network_rules.hpp"
#include <cstdio>
namespace nfs::game::audio {
using namespace state;
// Preserve playlist order; no extra custom-playlist capacity check is performed.
void read_catalog(int playlist) {
    Library &catalog = library();
    char previous_directory[260], playlist_path[260], section[44], relative_directory[260];
    GetCurrentDirectoryA(260, previous_directory);
    ini::Reader reader;
    if (!playlist) {
        std::sprintf(catalog.directory, "%smusic.ini", path(1044));
        ini::reader_construct(&reader, catalog.directory, 0);
        ini::get_int(&reader, "music", "numtunes", &catalog.track_count);
        ini::get_int(&reader, "music", "numvocaltunes", &catalog.vocal_count);
        ini::get_int(&reader, "music", "numinsttunes", &catalog.instrumental_count);
        ini::get_int(&reader, "music", "firstinst", &catalog.first_instrumental);
        for (int track = 0; track < catalog.track_count && track < 50; ++track) {
            std::sprintf(section, "track%d", track);
            std::sprintf(catalog.paths[track], "%strack%d.asf", path(1044), track);
            ini::get_string(&reader, section, "name", catalog.tracks[track].title, 259);
            ini::get_string(&reader, section, "artist", catalog.tracks[track].artist, 259);
            ini::get_string(&reader, section, "album", catalog.tracks[track].album, 259);
            catalog.played[track] = 0;
        }
    } else {
        std::sprintf(playlist_path, "%splaylist.ini", graphics::startup::configuration_path());
        std::sprintf(section, "playlist%d", playlist - 1);
        ini::reader_construct(&reader, playlist_path, 0);
        ini::get_string(&reader, section, "name", relative_directory, 259);
        std::sprintf(catalog.directory, "%s\\%s", graphics::startup::configuration_path(),
                     relative_directory);
        if (SetCurrentDirectoryA(catalog.directory)) {
            files::DirectorySearch search;
            int count = 0;
            if (files::find_first("*.asf", &search)) {
                do {
                    std::sprintf(catalog.paths[count], "%s\\%s", catalog.directory, search.name);
                    ++count;
                } while (files::find_next(&search));
            }
            files::find_close(&search);
            catalog.instrumental_count = 0;
            catalog.track_count = count;
            catalog.vocal_count = count;
        }
    }
    ini::reader_clear(&reader);
    SetCurrentDirectoryA(previous_directory);
}
} // namespace nfs::game::audio
