#include "nfs/input/window/window_input.hpp"
namespace nfs::window_input {

void key_queue_capacity(int capacity) {
    key_capacity = capacity;
    key_read = key_write = 0;
}

// The source reads the write index before taking the lock.
void enqueue_key(std::uint16_t character, Byte scan) {
    int write_index = key_write;
    kernel::lock_enter(&keyboard_lock);
    auto *entry = reinterpret_cast<Byte *>(&key_queue[write_index]);
    *reinterpret_cast<std::uint16_t *>(entry + 2) = character;
    entry[0] = scan;
    key_write = (key_write + 1) % key_capacity;
    if (key_read == key_write)
        key_read = (key_read + 1) % key_capacity;
    kernel::lock_leave(&keyboard_lock);
}

// EDX (the caller's repeat count) is preserved by this leaf.
Dword navigation_key(Dword message, Dword key) {
    if (message != 260 && message != 256)
        return 0;
    switch (key) {
    case 33:
    case 105:
        return 18688;
    case 34:
    case 99:
        return 20736;
    case 35:
    case 97:
        return 20224;
    case 36:
    case 103:
        return 18176;
    case 37:
    case 100:
        return 19200;
    case 38:
    case 104:
        return 18432;
    case 39:
    case 102:
        return 19712;
    case 40:
    case 98:
        return 20480;
    case 45:
    case 96:
        return 20992;
    case 46:
        return 21248;
    case 101:
        return 19456;
    case 112:
        return 15104;
    case 113:
        return 15360;
    case 114:
        return 15616;
    case 115:
        return 15872;
    case 116:
        return 16128;
    case 117:
        return 16384;
    case 118:
        return 16640;
    case 119:
        return 16896;
    case 120:
        return 17152;
    case 121:
        return 17408;
    case 122:
        return 34560;
    case 123:
        return 34816;
    default:
        return 0;
    }
}

} // namespace nfs::window_input
