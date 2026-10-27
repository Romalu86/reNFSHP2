#pragma once
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
namespace nfs::game::runtime::particles::lifecycle {
extern Byte particle_prefix[4868], particle_suffix[877];
Byte *state_address(unsigned);
template <class T> T &state(unsigned address) {
    return *reinterpret_cast<T *>(state_address(address));
}
Dword *construct_particle_buffer(Dword *);
Dword *construct_renderer(Dword *);
void destroy_renderer(Dword *);
void release_pool_image(Dword *);
void load_pool_file(Dword *, const char *);
void load_level(Dword *);
void unload_level(Dword *);
void initialize_car_effects();
void release_car_textures();
void release_wheel_skid(void *, int);
void mark_skid_finished(void *);
void advance_emitter_clock(void *, void *, float);
struct TextureScratch {
    graphics::TextureReader *owner;
};
TextureScratch *acquire_texture_scratch(TextureScratch *, graphics::TextureReader *);
void release_texture_scratch(TextureScratch *);
void *texture_scratch_data(TextureScratch *);
Dword texture_scratch_capacity(TextureScratch *);
Dword load_fsh_file(const char *, Dword *, Dword **, Dword);
void perpendicular_vector(const float *, float *);
void normalized_cross(const float *, const float *, float *);
void spread_direction(const float *, float, float *);
void *emit_car_particle(void *, const void *, float, const float *, const float *, const float *,
                        Dword, Dword);
namespace pending {
extern const Dword render_item_methods[];
extern const Dword render_item_base_methods[];
void load_car_particle_textures();
void load_car_particle_definitions();
void *load_fsh(const char *, Dword *, Dword **, Dword);
void *convert_fsh_image(graphics::TextureReader *, const void *, Dword);
Dword diagnostic_message(const char *);
void emit_type_particle(void *, void *, const float *, float);
} // namespace pending
} // namespace nfs::game::runtime::particles::lifecycle
