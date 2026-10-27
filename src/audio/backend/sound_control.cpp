#include "nfs/audio/backend/sound_runtime.hpp"
namespace nfs::sound {
Control control{};
HANDLE mutex = nullptr;
Byte effect_groups[84]{};

void create_mutex() {
    mutex = CreateMutexA(nullptr, FALSE, nullptr);
}

void close_mutex() {
    CloseHandle(mutex);
}

void wait_mutex() {
    WaitForSingleObject(mutex, INFINITE);
}

void release_mutex() {
    ReleaseMutex(mutex);
}

void lock() {
    wait_mutex();
    ++control.lock_depth;
}

void unlock() {
    --control.lock_depth;
    release_mutex();
}

int initialized() {
    return static_cast<std::int8_t>(control.initialized);
}

int find_voice(Dword handle) {
    if (static_cast<int>(handle) < 0)
        return -8;
    const Dword index = handle & 255;
    if (control.voice_count <= static_cast<int>(index))
        return -8;
    Voice *voice = control.voices + index;
    if (field<Byte>(voice, 93) == 0 || field<Dword>(voice, 0) != handle)
        return -8;
    return static_cast<int>(index);
}

bool next_voice(int first, int *cursor) {
    const std::int8_t group = field<std::int8_t>(control.voices + first, 34);
    if (!group) {
        if (*cursor < 0) {
            *cursor = first;
            return true;
        }
    } else {
        ++*cursor;
        while (*cursor < control.voice_count) {
            Voice *voice = control.voices + *cursor;
            if (field<std::int8_t>(voice, 34) == group && field<Byte>(voice, 93) == 1 &&
                field<int>(voice, 0) >= 0)
                return true;
            ++*cursor;
        }
    }
    return false;
}

void add_producer(Callback callback) {
    if (control.initialized) {
        field<Callback>(&control, 16 + 4 * static_cast<int>(control.producer_count)) = callback;
        ++control.producer_count;
    }
}

void remove_producer(Callback callback) {
    if (!control.initialized)
        return;
    lock();
    int index = 0;
    if (control.producer_count > 0) {
        while (field<Callback>(&control, 16 + 4 * index) != callback) {
            ++index;
            if (control.producer_count <= index) {
                unlock();
                return;
            }
        }
        --control.producer_count;
        for (; index < control.producer_count; ++index)
            field<Callback>(&control, 16 + 4 * index) =
                field<Callback>(&control, 20 + 4 * index);
    }
    unlock();
}

void add_pump(Callback callback) {
    field<Callback>(&control, 40 + 4 * static_cast<int>(control.pump_count)) = callback;
    ++control.pump_count;
}

void remove_pump(Callback callback) {
    int index = 0;
    if (control.pump_count > 0) {
        while (field<Callback>(&control, 40 + index * 4) != callback) {
            ++index;
            if (control.pump_count <= index)
                return;
        }
        --control.pump_count;
        for (; index < control.pump_count; ++index)
            field<Callback>(&control, 40 + index * 4) =
                field<Callback>(&control, 44 + index * 4);
    }
}

void pump() {
    if (control.initialized && control.pump_count > 0)
        for (int index = 0; index < control.pump_count; ++index)
            field<Callback>(&control, 40 + 4 * index)();
}

void *effect_group(int group, Dword flags) {
    if ((flags & 1820) == 4)
        return effect_groups + group * 20;
    if ((flags & 1820) != 16)
        return nullptr;
    return effect_groups + 20 + group * 20;
}

Dword effect_mask(Byte flags) {
    Dword mask = (flags & 16) ? 4 : 0;
    if (flags & 128)
        mask |= 16;
    return mask;
}
} // namespace nfs::sound
