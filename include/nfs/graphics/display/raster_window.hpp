#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::graphics::raster {
using kernel::Dword;
using Byte = std::uint8_t;
struct Window {
    Dword signature;
    int width, height;
    Byte type, depth, bits, shift;
    Byte target, padding[3];
    int left, top, right, bottom;
    Dword pitch;
    Dword *shape;
    Byte *pixels;
};
static_assert(sizeof(Window) == 48 && offsetof(Window, target) == 16);
struct State {
    Window current;
    Dword flags[12];
};
static_assert(sizeof(State) == 96);
extern Window current;
extern Dword flags[12];
extern Dword selected_depth, locked, active_lock;
extern Window *fallback;
extern Dword *buffer;
using LockBuffer = Dword *(__cdecl *)();
using UnlockBuffer = void(__cdecl *)(Dword *);
using SelectTarget = void(__cdecl *)(Byte);
extern LockBuffer lock_buffer;
extern UnlockBuffer unlock_buffer;
extern SelectTarget select_target;
Dword shape_type(Dword);
Dword *allocate_window_storage(int, int, int, Dword, Dword);
Dword *allocate_window(int, int, int, Dword);
Byte lock(Window *);
void unlock();
Byte select(Window *);
void disable();
void save(State *);
void restore(const State *);
} // namespace nfs::graphics::raster
