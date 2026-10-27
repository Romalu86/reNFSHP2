#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio {

void pending::load_race_banks() {
    using namespace state;
    using namespace vehicle;
    effects_storage[0] = 1;
    if (!configuration<Dword>(audio_layout::audio_enabled))
        return;
    auto cars = []() { return runtime::system(68); };
    char car_file[260], archive[260], filename[276];
    int kind;
    auto generic = [&](const char *name, int bank_index) {
        std::strcpy(filename, path(1304));
        std::strcat(filename, name);
        load_bank(filename, bank_index, lifecycle::audio_pool | 1024);
    };
    if (at<int>(cars(), 8) < 2)
        generic("gencar", 1);
    else {
        Progress progress;
        construct_progress(&progress, at<int>(cars(), 8));
        // This stack owner supplies the shipped exception-unwind destructor as well.
        struct Scope {
            Progress *value;
            ~Scope() {
                destroy_progress(value);
            }
        } scope{&progress};
        for (Dword index = 0; index < at<Dword>(cars(), 8); ++index) {
            kernel::cooperate(0);
            unsigned offset = 20 + index * 4;
            void *car = at<void *>(cars(), offset);
            if (car && offset < 116) {
                std::sprintf(car_file, "%s%s\\car.ini", path(2084),
                             reinterpret_cast<char *>(car) + 1532);
                ini::Reader reader;
                ini::reader_construct(&reader, car_file, 0);
                ini::get_int(&reader, "audio", "enginetype", &kind);
                ini::reader_clear(&reader);

                std::strcpy(filename, configuration<CarSound>(audio_layout::car_sounds + kind * 52).engine);
                std::strcat(filename, ".bnk");
                std::sprintf(archive, "%soppbnk.viv", path(1304));
                bool shared = false;
                for (Dword previous = 0; previous < index; ++previous)
                    if (banks()[32 + previous].vehicle_kind == kind) {
                        banks()[32 + index].handle = banks()[32 + previous].handle;
                        banks()[32 + index].vehicle_kind = banks()[32 + previous].vehicle_kind;
                        banks()[32 + index].data = nullptr;
                        shared = true;
                        break;
                    }
                if (shared ||
                    load_archive_bank(archive, filename, index + 32, lifecycle::audio_pool | 1024))
                    banks()[32 + index].vehicle_kind = kind;
                else {
                    generic("genopp", index + 32);
                    banks()[32 + index].vehicle_kind = -1;
                }
            }
            advance_progress(&progress);
        }
        generic("gencar", 1);
    }
    for (Dword index = 0; index < at<Dword>(cars(), 8); ++index) {
        kernel::cooperate(0);
        unsigned offset = 20 + index * 4;
        void *car = at<void *>(cars(), offset);
        if (!car || offset >= 116)
            continue;
        int horn = 99;
        std::sprintf(car_file, "%s%s\\car.ini", path(2084), reinterpret_cast<char *>(car) + 1532);
        ini::Reader reader;
        ini::reader_construct(&reader, car_file, 0);
        ini::get_int(&reader, "audio", "horntype", &horn);
        ini::reader_clear(&reader);
        if (horn < 16) {
            std::strcpy(filename, &configuration<char>(audio_layout::horn_names + horn * 16));
            std::strcat(filename, ".bnk");
        } else
            std::strcpy(filename, "genhorn.bnk");
        std::sprintf(archive, "%shornz.viv", path(1304));
        int bank_index;
        if (index == at<Dword>(runtime::system(72), 204))
            bank_index = 29;
        else if (index == at<Dword>(runtime::system(72), 208) && at<int>(cars(), 16) == 2)
            bank_index = 30;
        else
            bank_index = index + 13;
        if (!load_archive_bank(archive, filename, bank_index, lifecycle::audio_pool | 1024))
            load_archive_bank(archive, "genhorn.bnk", bank_index, lifecycle::audio_pool | 1024);
    }
    for (unsigned volume_offset : {static_cast<unsigned int>( audio_layout::player_volume ), static_cast<unsigned int>( audio_layout::second_player_volume )})
        if (configuration<int>(volume_offset) > configuration<int>(audio_layout::car_volume_limit))
            configuration<int>(volume_offset) = configuration<int>(audio_layout::car_volume_limit);
    int listeners = at<int>(runtime::system(72), 176) == 2 ? 2 : 1;
    for (int listener = 0; listener < listeners; ++listener) {
        void *car = runtime::physics::local_car(cars(), listener);
        option<int>(audio_layout::engine_type) = 0;
        if (car) {
            std::sprintf(car_file, "%s%s\\car.ini", path(2084),
                         reinterpret_cast<char *>(car) + 1532);
            ini::Reader reader;
            ini::reader_construct(&reader, car_file, 0);
            ini::get_int(&reader, "audio", "enginetype", &option<int>(audio_layout::engine_type));
            ini::reader_clear(&reader);
        }
        initialize_engine_owner(option<int>(audio_layout::engine_type), listener);
    }
}
void pending::release_track_audio() {
    vehicle::release_engine_owners();
}
} // namespace nfs::game::audio
