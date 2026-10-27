#include "nfs/audio/speech/speech_game.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
namespace nfs::speech::game {
namespace audio = nfs::game::audio;
namespace {
struct ReaderScope {
    ini::Reader reader;
    explicit ReaderScope(const char *path) {
        ini::reader_construct(&reader, path, 0);
    }
    ~ReaderScope() {
        ini::reader_clear(&reader);
    }
};
} // namespace

void load_bank_catalog(Owner *owner) {
    char filename[260], name[88], archive[64], location[64], key[16];
    int count = 0, index = 0;
    std::sprintf(filename, "%s%s\\spchbnks.ini", audio::state::path(1564),
                 languages[owner->language]);
    ReaderScope general(filename);
    ini::get_int(&general.reader, "speech", "numbanks", &count);
    for (int bank = 0; bank < count; ++bank) {
        std::sprintf(key, "bank%d", bank);
        ini::get_string(&general.reader, key, "bank", name, 63);
        ini::get_string(&general.reader, key, "viv", archive, 63);
        load_bank(owner, name, index++, archive, archive);
    }
    std::sprintf(filename, "%s%s\\location.ini", audio::state::path(1564),
                 languages[owner->language]);
    ReaderScope locations(filename);
    ini::get_int(&locations.reader, "locations", "numloc", &count);
    const char *archives[] = {"19loc", "27loc", "31loc", "46loc", "rtrloc", "disploc"};
    const char *groups[] = {"19locations", "27locations",  "31locations",
                            "46locations", "rtrlocations", "dlocations"};
    for (int voice = 0; voice < 6; ++voice) {
        for (int local = 0; local < count; ++local) {
            std::sprintf(key, "loc%d", local);
            ini::get_string(&locations.reader, "locations", key, location, 63);
            std::sprintf(archive, "%s", archives[voice]);
            if (voice < 3) {
                const int language = sound::field<int>(lifecycle::manager->slots[72 / 4], 2312);
                const char *pattern = language == 1 ? "%s"
                                                    : (voice == 0   ? "19%s"
                                                       : voice == 1 ? "27%s"
                                                                    : "31%s");
                std::sprintf(name, pattern, location);
            } else if (voice == 3)
                std::sprintf(name, "%s", location);
            else
                std::sprintf(name, voice == 4 ? "%sR" : "%sD", location);
            load_bank(owner, name, index++, archive, groups[voice]);
        }
    }
}
} // namespace nfs::speech::game
