#pragma once
#include "nfs/graphics/resources/eagl_objects.hpp"
namespace nfs::graphics::viewports {
extern std::uint8_t switching;
void rectangle(Dword *, float, float, float, float, float, float);
void read_rectangle(const Dword *, float *, float *, float *, float *, Dword *, Dword *);
void set_matrix(Dword *, const float *);
void refresh(Dword *);
void perspective(Dword *, float, float, float, float);
void orthographic(Dword *, float, float, float);
void pixel_orthographic(Dword *, float, float);
void begin(Dword *);
void end(Dword *);
void clear(Dword *, std::uint8_t);
void target_dimensions(const Dword *, float *, float *);
Dword *target_viewport(const Dword *);
void set_target_viewport(Dword **, Dword *);
} // namespace nfs::graphics::viewports
namespace nfs::graphics::backend {
extern Dword screen_info[4];
}
