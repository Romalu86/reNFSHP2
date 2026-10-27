#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::sound {

void retire_voice(int index) {
    Voice *voice = control.voices + index;
    auto retire = [](Voice *item) {
        field<volatile Byte>(item, 93) = 0;
        field<volatile Byte>(item, 34) = 0;
        field<volatile Byte>(item, 35) = 0;
        field<volatile Dword>(item, 40) = static_cast<volatile Dword &>(control.unknown_c);
    };
    if (field<Byte>(voice, 34) == 0) {
        retire(voice);
        return;
    }
    int count = 0, leader = -1;
    for (int i = 0; i < control.voice_count; ++i) {
        Voice *item = control.voices + i;
        if (field<Byte>(item, 34) == field<Byte>(voice, 34) && field<int>(item, 0) >= 0 &&
            field<Byte>(item, 93) != 0) {
            ++count;
            if (field<Byte>(item, 35) != 0)
                leader = i;
        }
    }
    if (count != 1) {
        Voice *item = control.voices + leader;
        Byte status = field<Byte>(item, 93);
        if (status == 2 && index != leader && count == 2) {
            retire(voice);
            field<volatile Byte>(static_cast<Voice *volatile &>(control.voices) + leader, 93) = 0;
            field<volatile Byte>(static_cast<Voice *volatile &>(control.voices) + leader, 34) = 0;
            field<volatile Byte>(static_cast<Voice *volatile &>(control.voices) + leader, 35) = 0;
            Voice *retired = static_cast<Voice *volatile &>(control.voices) + leader;
            field<volatile Dword>(retired, 40) = static_cast<volatile Dword &>(control.unknown_c);
            return;
        }
        if (status == 1 && index == leader) {
            field<Byte>(item, 93) = 2;
            return;
        }
    }
    retire(voice);
}

void clear_backend_chain(void *buffer) {
    BackendVoice *backend = backends + field<std::int16_t>(buffer, 2);
    while (void *node = field<void *>(backend, 212)) {
        void *next = field<void *>(node, 8);
        auto *base = reinterpret_cast<Byte *>(backend);
        if (node != base + 28 && node != base + 100) {
            using Destroy = void(__cdecl *)(void *);
            auto destroy = field<Destroy>(node, 4);
            if (destroy)
                destroy(node);
            release(field<void *>(backend, 212));
        }
        field<void *>(backend, 212) = next;
    }
}

int stop_voice(int index) {
    Voice *voice = control.voices + index;
    BackendVoice *backend = backends + index;
    auto *bytes = reinterpret_cast<Byte *>(backend);
    backend_call<void>(backend, 216, bytes + 28);
    const auto flags = field<std::uint16_t>(voice, 32);
    if (!(flags & 1)) {
        if (flags & 2)
            clear_backend_chain(bytes + 100);
    } else
        field<Dword>(backend, 256) = 0;
    if (field<Byte>(voice, 32) & 16)
        retire_voice(index);
    return 0;
}

int stop(Dword handle) {
    if (!control.initialized)
        return -10;
    int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor))
            stop_voice(cursor);
    }
    return first;
}

void stop_all() {
    lock();
    for (int index = 0; index < control.voice_count; ++index)
        stop(field<Dword>(control.voices + index, 0));
    unlock();
}
} // namespace nfs::sound
