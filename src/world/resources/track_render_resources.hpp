#pragma once
#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
namespace nfs::game::world::track_render {
struct TextureOptions {
    graphics::TextureReader *reader;
    Byte flags[6];
};
static_assert(sizeof(TextureOptions) == 12);
TextureOptions *begin_texture_options(TextureOptions *, graphics::TextureReader *);
void end_texture_options(TextureOptions *);
eagl::SymbolView *first_model(eagl::SymbolView *, eagl::Loader *);
eagl::SymbolView *find_model(eagl::SymbolView *, eagl::Loader *, const char *, Byte);
Dword *first_model_property(void *, const char *);
Dword *last_model_property(void *, const char *);
void configure_track_states(void *);
void configure_boom_states(void *);
void enable_mirror_indices(void *);
Dword *find_render_layer(void *, const char *);
void bind_compartment_model(void *, eagl::Loader *);
void bind_directory_model(void *, eagl::Loader *);
void set_alpha_references(void *);
eagl::Loader *link_track_module(void *, Dword, void *, eagl::Resolve);
void *load_persistent_fsh(graphics::TextureReader *, const char *, const char *);
void *construct_sun_glow(void *);
void bind_sun_glow(void *, const float *, void *, float, const float *);
void construct_compartment(void *, Dword);
extern float glow_distance, glow_fade, glow_scale, glow_distance_secondary, glow_fade_secondary,
    glow_scale_secondary;
} // namespace nfs::game::world::track_render
