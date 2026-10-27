#include "nfs/graphics/display/raster_window.hpp"
#include "nfs/graphics/textures/shape_conversion.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstdio>
#include <cstring>

namespace nfs::graphics::raster {
Window current = {};
Dword flags[12] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
Dword selected_depth = 0, locked = 0, active_lock = 0;
Window *fallback = nullptr;
Dword *buffer = nullptr;
LockBuffer lock_buffer = nullptr;
UnlockBuffer unlock_buffer = nullptr;
SelectTarget select_target = nullptr;
namespace {
Byte depth_shift(int bits) {
    float value = static_cast<float>(bits);
    Dword encoded;
    std::memcpy(&encoded, &value, 4);
    return Byte((encoded >> 23) + 126);
}
} // namespace

Dword shape_type(Dword format) {
    switch (format & 2147483647) {
    case 1:
        return 121;
    case 2:
        return 123;
    case 3:
    case 11:
        return 126;
    case 4:
        return 120;
    case 5:
        return 127;
    case 6:
        return 125;
    case 7:
        return 109;
    case 12:
        return 96;
    case 13:
        return 97;
    case 14:
        return 98;
    case 23:
        return 100;
    case 26:
        return 101;
    default:
        return 0;
    }
}
// Window allocation accepts width, height, depth, flags and pool.
 
Dword *allocate_window_storage(int width, int height, int depth, Dword allocation_flags, Dword pool) {
    (void)pool; // The allocator pool argument is unused.
    if (width <= 0 || width >= 65537 || height <= 0 || height >= 65537 ||
        static_cast<int>(Dword(width) * height) > 1000000000)
        return nullptr;
    if (!depth) {
        depth = selected_depth;
        if (!depth)
            depth = 16;
    }
    Dword type = textures::shape::type_from_depth(depth);
    int decoded_depth = tar::image_depth(&type), bits = decoded_depth == 15 ? 16 : decoded_depth;
    if (!type)
        return nullptr;
    int pitch = ((bits * width + 7) & ~7) / bits * bits >> 3;
    char label[16];
    std::sprintf(label, "%dx%dx%d", width, height, depth);
    auto *result = static_cast<Dword *>(kernel::memory_allocate(
        label, static_cast<int>(Dword(pitch) * height + 64), allocation_flags));
    if (!result)
        return nullptr;
    kernel::memory_zero(result, 64);
    result[12] = type;
    reinterpret_cast<std::int16_t *>(result + 13)[0] = std::int16_t(width);
    reinterpret_cast<std::int16_t *>(result + 13)[1] = std::int16_t(height);
    auto *window = reinterpret_cast<Window *>(result);
    window->signature = 1464421956;
    window->width = width;
    window->height = height;
    window->depth = Byte(decoded_depth);
    window->type = Byte(type);
    window->target = 0;
    window->bits = Byte(bits);
    window->shift = depth_shift(bits);
    window->left = window->top = 0;
    window->right = width;
    window->bottom = height;
    window->pitch = pitch;
    window->shape = result + 12;
    window->pixels = result[15] & 4096 ? reinterpret_cast<Byte *>(result + 12) + result[16]
                                         : reinterpret_cast<Byte *>(result + 16);
    return result;
}
// The passed zero is a pool selector, not a new initialization policy.
Dword *allocate_window(int width, int height, int depth, Dword allocation_flags) {
    return allocate_window_storage(width, height, depth, allocation_flags, kernel::default_pool);
}
// A missing buffer returns the initialized zero byte.
Byte lock(Window *window) {
    if (active_lock)
        return 1;
    Byte target = window->target;
    select_target(target);
    buffer = lock_buffer();
    if (!buffer)
        return 0;
    window->signature = 1464421956;
    window->width = buffer[3];
    window->height = buffer[4];
    Dword type = shape_type(buffer[2]);
    window->type = Byte(type);
    window->depth = tar::image_depth(&type);
    window->bits = window->depth == 15 ? 16 : window->depth;
    window->target = target;
    window->left = window->top = 0;
    window->shift = depth_shift(window->bits);
    window->right = buffer[3];
    window->bottom = buffer[4];
    window->pitch = buffer[1];
    window->shape = nullptr;
    window->pixels = reinterpret_cast<Byte *>(buffer[0]);
    return 1;
}

void unlock() {
    if (!active_lock && buffer) {
        unlock_buffer(buffer);
        locked = 0;
        if (current.target == 1)
            flags[11] = 0;
        else if (current.target == 2) {

            // Its shared backing is retained separately below.
            extern Dword secondary_target_lock;
            secondary_target_lock = 0;
        }
        buffer = nullptr;
        select_target(0);
    }
}
Dword secondary_target_lock = 0;

Byte select(Window *window) {
    Byte result = 1;
    unlock();
    if (!window) {
        kernel::memory_fill(&current, 0, 48);
        current.depth = reinterpret_cast<Byte *>(flags)[13];
        current.type = reinterpret_cast<Byte *>(flags)[12];
        return 1;
    }
    if (window->target) {
        result = lock(window);
        if (!result) {
            if (!fallback)
                fallback = reinterpret_cast<Window *>(allocate_window(8, 8, selected_depth, 0));
            kernel::memory_copy(&current, fallback, 48);
            return 0;
        }
    }
    kernel::memory_copy(&current, window, 48);
    return result;
}

void disable() {
    select(nullptr);
}

void save(State *state) {
    kernel::memory_copy(&state->current, &current, 48);
    kernel::memory_copy(state->flags, flags, 48);
}

void restore(const State *state) {
    kernel::memory_copy(flags, state->flags, 48);
    kernel::memory_copy(&current, &state->current, 48);
    if (current.target && !locked && select_target) {
        select_target(current.target);
        return;
    }
    select(&current);
}
} // namespace nfs::graphics::raster
