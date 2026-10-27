#pragma once
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::game::world::vehicle_render {
Byte attachment_enabled(void *, Dword);
void read_attachment_transform(void *, Dword, Dword *, Dword *);
Byte read_vehicle_attachment(void *, Dword, Dword *, Dword *);
void read_model_transform(const Dword *, Dword *, Dword *);
} // namespace nfs::game::world::vehicle_render
