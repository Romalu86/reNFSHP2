#include "nfs/audio/speech/speech_game.hpp"
#include "nfs/audio/speech/speech_queue.hpp"
#include "nfs/audio/speech/speech_rules.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstdio>
#include <cstring>

namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::speech::game {
namespace audio = nfs::game::audio;
namespace streams = sound::streaming;


void select_language(void *object) {
    auto *owner = static_cast<Owner *>(object);
    char path[260] = {};
    owner->language = sound::field<int>(lifecycle::manager->slots[18], 2312);
    std::sprintf(path, "%s%s\\events.evt", audio::state::path(1564), languages[owner->language]);
    if (!files::file::exists(path)) {
        for (int language = 0; language < 7; ++language) {
            std::sprintf(path, "%s%s\\events.evt", audio::state::path(1564), languages[language]);
            if (files::file::exists(path)) {
                owner->language = language;
                break;
            }
        }
    }
}


void post_initialize(void *owner) {
    select_language(owner);
    load_resources(static_cast<Owner *>(owner));
}


void shutdown_owner(void *object) {
    auto *owner = static_cast<Owner *>(object);
    for (unsigned index = 0; index < 450; ++index) {
        auto &bank = banks()[index];
        if (bank.table != -1) {
            phrase_table(bank.table);
            detach_phrase_table(bank.table);
        }
        bank.table = -1;
        bank.header = nullptr;
        bank.reserved = 0;
        bank.name[0] = 0;
    }
    if (owner->enabled)
        shutdown();
    const int handle = owner->stream;
    if (handle)
        streams::stop_stream(handle);
    if (owner->stream_memory) {
        streams::destroy_stream(handle);
        kernel::memory_free(owner->stream_memory);
    }
    owner->stream_memory = nullptr;
    if (owner->table_storage)
        kernel::memory_free(owner->table_storage);
    if (owner->directory)
        kernel::memory_free(owner->directory);
}


void load_level(void *object) {
    auto *owner = static_cast<Owner *>(object);
    const int shuffles = (network::rules::random() & 3) + 2;
    for (int shuffle = 0; shuffle < shuffles; ++shuffle) {
        const unsigned first = network::rules::random() % 6;
        const unsigned second = network::rules::random() % 6;
        Dword temporary = paired_voice_masks[first];
        paired_voice_masks[first] = paired_voice_masks[second];
        paired_voice_masks[second] = temporary;
        temporary = voice_masks[first];
        voice_masks[first] = voice_masks[second];
        voice_masks[second] = temporary;
        const unsigned first_dispatch = network::rules::random() & 3;
        const unsigned second_dispatch = network::rules::random() & 3;
        temporary = dispatch_masks[first_dispatch];
        dispatch_masks[first_dispatch] = dispatch_masks[second_dispatch];
        dispatch_masks[second_dispatch] = temporary;
    }
    sound::VoiceParameters parameters;
    sound::initialize_parameters(&parameters);
    const int bytes = static_cast<int>(Dword(streams::file_stream_bytes(26, 300)) + 194560);
    owner->stream_memory =
        kernel::memory_allocate("sndstream", bytes, lifecycle::audio_pool | 1280);
    if (!owner->stream_memory)
        owner->enabled = 0;
    else {
        owner->stream =
            streams::create_file_stream(&parameters, 26, 300, owner->stream_memory, bytes);
        if (owner->stream < 0) {
            kernel::memory_free(owner->stream_memory);
            owner->stream_memory = nullptr;
            owner->enabled = 0;
        }
    }
    sound::field<int>(owner, 48) = 10000;
    sound::field<int>(owner, 40) = 10000;
    sound::field<Dword>(owner, 44) = 0;
    sound::field<Byte>(owner, 89) = 0;
    sound::field<Dword>(owner, 36) = 0;
    sound::field<Dword>(owner, 60) = 0;
    sound::field<Byte>(owner, 53) = 0;
    sound::field<Byte>(owner, 52) = 0;
    sound::field<int>(owner, 76) = -1;
    sound::field<Dword>(owner, 80) = 0;
    sound::field<Byte>(owner, 88) = 0;
    sound::field<Byte>(owner, 90) = 0;
    sound::field<Byte>(owner, 91) = 0;
    sound::field<Byte>(owner, 92) = 0;
    sound::field<Byte>(owner, 94) = 0;
    sound::field<Byte>(owner, 93) = 0;
    sound::field<Byte>(owner, 95) = 0;
    sound::field<int>(owner, 68) = -1;
    sound::field<int>(owner, 64) = -1;
    sound::field<int>(owner, 72) = -1;
    owner->phrase_flags = 0;
    owner->request_kind = 22;
    sound::field<Dword>(owner, 56) = network::rules::random() % 3;
}


void unload_level(void *object) {
    auto *owner = static_cast<Owner *>(object);
    const int handle = owner->stream;
    if (handle)
        streams::stop_stream(handle);
    if (owner->stream_memory) {
        streams::destroy_stream(handle);
        kernel::memory_free(owner->stream_memory);
    }
    owner->stream_memory = nullptr;
}


void enter_frontend(void *object) {
    auto *owner = static_cast<Owner *>(object);
    sound::VoiceParameters parameters;
    sound::initialize_parameters(&parameters);
    const int bytes = static_cast<int>(Dword(streams::file_stream_bytes(26, 300)) + 194560);
    owner->stream_memory =
        kernel::memory_allocate("sndstream", bytes, lifecycle::audio_pool | 1280);
    if (!owner->stream_memory) {
        owner->enabled = 0;
        return;
    }
    owner->stream = streams::create_file_stream(&parameters, 26, 300, owner->stream_memory, bytes);
    if (owner->stream < 0) {
        kernel::memory_free(owner->stream_memory);
        owner->stream_memory = nullptr;
        owner->enabled = 0;
    }
}


void leave_frontend(void *object) {
    auto *owner = static_cast<Owner *>(object);
    const int handle = owner->stream;
    if (handle)
        streams::stop_stream(handle);
    if (owner->stream_memory) {
        streams::destroy_stream(handle);
        kernel::memory_free(owner->stream_memory);
    }
    owner->stream_memory = nullptr;
}


void reset_race(void *object) {
    auto *owner = static_cast<Owner *>(object);
    sound::field<int>(owner, 48) = 10000;
    sound::field<int>(owner, 40) = 10000;
    sound::field<Dword>(owner, 44) = 0;
    sound::field<Dword>(owner, 36) = 0;
    sound::field<Dword>(owner, 60) = 0;
    sound::field<Byte>(owner, 53) = 0;
    sound::field<Byte>(owner, 52) = 0;
    sound::field<int>(owner, 76) = -1;
    sound::field<Dword>(owner, 80) = 0;
    sound::field<Byte>(owner, 88) = 0;
    sound::field<Byte>(owner, 90) = 0;
    sound::field<Byte>(owner, 91) = 0;
    sound::field<Byte>(owner, 92) = 0;
    sound::field<Byte>(owner, 94) = 0;
    sound::field<Byte>(owner, 93) = 0;
    sound::field<Byte>(owner, 95) = 0;
    sound::field<Byte>(owner, 89) = 0;
    sound::field<int>(owner, 68) = -1;
    sound::field<int>(owner, 64) = -1;
    sound::field<int>(owner, 72) = -1;
    owner->phrase_flags = 0;
    owner->request_kind = 22;
    const Dword variation = network::rules::random() % 3;
    audio::state::configuration<Dword>(audio_layout::opponent_volume) = audio::state::configuration<Dword>(audio_layout::baseline_opponent_volume);
    audio::state::configuration<Dword>(12) = 0;
    sound::field<Dword>(owner, 56) = variation;
    audio::state::configuration<Dword>(audio_layout::player_volume) = audio::state::configuration<Dword>(audio_layout::baseline_player_volume);
    audio::apply_event_volumes();
    set_engines_disabled(0);
}


void set_engines_disabled(Byte disabled) {
    nfs::game::runtime::race_audio::pending::engines_disabled = disabled;
}
} // namespace nfs::speech::game
