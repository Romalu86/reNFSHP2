#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::game::audio::vehicle {

int initialize_engine_owner(int kind, int listener) {
    using namespace state;
    if (!configuration<Dword>(audio_layout::audio_enabled) || owner(listener))
        return 0;
    owner(listener) = static_cast<EngineOwner *>(
        kernel::memory_allocate("Engine Audio", 444, lifecycle::audio_pool | 1024));
    kernel::memory_zero(owner(listener), 444);
    at<int>(owner(listener), 36) = at<int>(owner(listener), 40) = 44;
    for (int i = 0; i < 16; ++i)
        at<int>(owner(listener), 52 + i * 12) = at<int>(owner(listener), 244 + i * 12) = -1;
    table(7218888, listener) = table(7218896, listener) = table(7218880, listener) = nullptr;
    auto &car = configuration<CarSound>(audio_layout::car_sounds + kind * 52);
    char archive[260];
    std::sprintf(archive, "%s%s.viv", path(1304), car.bank);
    for (int i = 0; i < 24; ++i) {
        bank(listener, i).handle = -1;
        bank(listener, i).pitch = 0;
    }
    at<int>(owner(listener), 28) = car.bend_range;
    at<int>(owner(listener), 32) = car.bass_add;
    at<int>(owner(listener), 36) = car.inside;
    at<int>(owner(listener), 40) = car.outside;
    char engine[8]{}, exhaust[8]{};
    std::strcpy(engine, car.engine);
    std::strcpy(exhaust, car.exhaust);
    load_patch_group(archive, engine, 100, listener);
    load_patch_group(archive, engine, 101, listener);
    load_patch_group(archive, exhaust, 102, listener);
    load_patch_group(archive, exhaust, 103, listener);
    load_patch_group(archive, exhaust, 108, listener);
    int volume = car.volume;
    if (!volume) {
        char filename[260];
        std::sprintf(filename, "%svolumes.ini", path(1304));
        ini::Reader reader;
        ini::reader_construct(&reader, filename, 0);
        ini::get_int(&reader, "volume", "player", &volume);
        ini::reader_clear(&reader);
    }
    configuration<int>(audio_layout::player_volume) = configuration<int>(audio_layout::second_player_volume) = volume < 0     ? 0
                                                                  : volume > 127 ? 127
                                                                                 : volume;
    configuration<int>(audio_layout::baseline_player_volume) = configuration<int>(audio_layout::player_volume);
    configuration<int>(audio_layout::baseline_second_player_volume) = configuration<int>(audio_layout::second_player_volume);
    initialize_volume_fades();
    const char *names[3] = {"engine.ltb", "engine.ctb", "engine.rpm"};
    void *loaded[3];
    std::sprintf(archive, "%scartable.viv", path(1304));
    files::archives::load_many(archive, names, loaded, 3, lifecycle::audio_pool | 1024);
    table(7218896, listener) = static_cast<Byte *>(loaded[0]);
    table(7218888, listener) = static_cast<Byte *>(loaded[1]);
    table(7218880, listener) = static_cast<Byte *>(loaded[2]);
    if (loaded[0] && loaded[1]) {
        bind_engine_tables(listener);
        Byte *rpm = table(7218880, 0); // Code always relocates listener zero's RPM resource.
        if (!rpm[5]) {
            for (unsigned offset = 328; offset < 360; offset += 4) {
                at<Dword>(rpm, offset - 32) += reinterpret_cast<Dword>(rpm + offset - 32);
                at<Dword>(rpm, offset) += reinterpret_cast<Dword>(rpm + offset);
            }
            rpm[5] = 1;
        }
        int listeners = at<int>(runtime::system(72), 176);
        if (listeners == 1 || (listeners == 2 && listener == 1))
            sound::add_producer(mix_engine_owners);
    }
    return 1;
}

void release_engine_owners() {
    if (!state::configuration<Dword>(audio_layout::audio_enabled))
        return;
    sound::lock();
    for (int listener = 0; listener < 2; ++listener) {
        auto *engine = owner(listener);
        if (!engine)
            break;
        at<Dword>(engine, 4) = 0;
        for (unsigned offset = 0; offset < 192; offset += 12)
            if (at<int>(owner(listener), 52 + offset) != -1) {
                sound::stop(at<Dword>(owner(listener), 52 + offset));
                at<int>(owner(listener), 52 + offset) = -1;
                sound::stop(at<Dword>(owner(listener), 244 + offset));
                at<int>(owner(listener), 244 + offset) = -1;
            }
        kernel::memory_free(table(7218904, listener * 2));
        kernel::memory_free(table(7218904, listener * 2 + 1));
        kernel::memory_free(owner(listener));
        owner(listener) = nullptr;
        for (int patch = 0; patch < 24; ++patch) {
            auto &entry = bank(listener, patch);
            if (entry.data) {
                sound::release_bank(entry.handle);
                entry.handle = -1;
                kernel::memory_free(entry.data);
                entry.data = nullptr;
            }
        }
        if (table(7218880, listener))
            kernel::memory_free(table(7218880, listener));
    }
    sound::remove_producer(mix_engine_owners);
    sound::unlock();
}
} // namespace nfs::game::audio::vehicle
