#include "nfs/audio/speech/speech_game.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/resources/archives/big_directory.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/core/timing/timer.hpp"
#include <cstdio>
#include <cstring>
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::speech::game {
namespace audio = nfs::game::audio;
// The third stack argument is unused, and the return pops twelve bytes.
Dword play_phrase(Owner *owner, int table, int offset, int) {
    if (!owner->enabled)
        return 0;
    for (unsigned index = 0; index < 450; ++index) {
        auto &bank = banks()[index];
        if (bank.table != table)
            continue;
        const Dword stream = Dword(owner->stream);
        Dword result = sound::streaming::enqueue_file(stream, 0, bank.name,
                                                     int(bank.data_offset + Dword(offset)));
        int volume = nfs::game::runtime::race_audio::group_volume(127, 3, 127);
        if (audio::state::library().selection == -1) {
            volume = int(static_cast<long double>(volume) * 1.2000000476837158203125f);
            if (volume < 0)
                volume = 0;
            else if (volume > 127)
                volume = 127;
        }
        sound::lock();
        nfs::game::runtime::race_audio::set_stream_volume(stream, volume);
        sound::unlock();
        return result;
    }
    return 0;
}

void __cdecl emit_phrase(const PhrasePlayback *phrase) {
    play_phrase(reinterpret_cast<Owner *>(lifecycle::manager->slots[84 / 4]), phrase->table_index,
                phrase->sample_offset, phrase->sample_length);
}

void load_bank(Owner *owner, const char *name, int index, const char *archive_name,
               const char *header_group) {
    if (!owner->directory)
        return;
    char archive[256], header[256], data_name[256];
    std::sprintf(archive, "%s%s\\%s.viv", audio::state::path(1564), languages[owner->language],
                 archive_name);
    std::sprintf(header, "files\\%s\\%s.hdr", header_group, name);
    Dword offset, bytes;
    if (!big_find_entry(static_cast<Byte *>(owner->directory), header, 0, &offset, &bytes))
        return;
    auto &bank = banks()[index];
    bank.header = static_cast<Byte *>(owner->directory) + offset;
    std::strcpy(bank.name, archive);
    std::sprintf(data_name, "%s.dat", name);
    bank.data_offset = files::archives::offset(archive, data_name, lifecycle::audio_pool | 1280);
    bank.table = register_phrase_table(bank.header);
}

void load_resources(Owner *owner) {
    if (!audio::state::configuration<Dword>(audio_layout::audio_enabled)) {
        owner->enabled = 0;
        return;
    }
    for (unsigned index = 0; index < 450; ++index) {
        auto &bank = banks()[index];
        bank.header = nullptr;
        bank.reserved = 0;
        bank.table = -1;
        bank.name[0] = 0;
    }
    const Dword format = Dword(scale_speech_parameter(24000, 16, 1));
    initialize(reinterpret_cast<Dword>(emit_phrase), 0, format);
    Dword storage_bytes = phrase_table_storage_size(450);
    owner->table_storage =
        kernel::memory_allocate("bank_mem", storage_bytes, lifecycle::audio_pool);
    initialize_phrase_tables(450, static_cast<Byte **>(owner->table_storage));
    Dword *callbacks = reinterpret_cast<Dword *>(speech_configuration());
    callbacks[0] = callbacks[1] = reinterpret_cast<Dword>(kernel::print::check_console_write);
    callbacks[2] = reinterpret_cast<Dword>(kernel::timer_count);
    const int language = owner->language;
    char filename[260];
    std::sprintf(filename, "%s%s\\events.evt", audio::state::path(1564), languages[language]);
    auto *events =
        static_cast<Byte *>(files::file::try_load(filename, lifecycle::audio_pool | 1024));
    kernel::print::check_console_write("done loadfileadr\n");
    if (events && register_speech_rule(events, 0)) {
        char directory[256];
        std::sprintf(directory, "%s%s\\speechdr.viv", audio::state::path(1564),
                     languages[language]);
        owner->directory = files::file::try_load(directory, lifecycle::audio_pool | 1024);
        load_bank_catalog(owner);
        owner->enabled = 1;
    } else
        owner->enabled = 0;
}
} // namespace nfs::speech::game
