#pragma once
#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::immediate {
extern backend::TextureStageState gouraud_stages, textured_stages;
Dword *dynamic_model_construct(Dword *);
void dynamic_model_destroy(Dword *);
void dynamic_model_matrix(Dword *, const Dword *);
Dword *model_builder_construct(Dword *);
void model_builder_destroy(Dword *);
std::uint8_t stop_model_builder(Dword *);
std::uint8_t model_builder_value(Dword *, Dword);
std::uint8_t model_builder_matrix(Dword *, const Dword *);
void model_builder_bindings(Dword *, Dword);
std::uint8_t render_model_builder(Dword *, Dword);
void submit_dynamic_model(Dword *);
void draw_dynamic_model(Dword *);
void draw_array(Dword *, int);
int append_dynamic_primitive(Dword *, Dword *);
std::uint8_t bind_builder_primitive(Dword *, Dword *);
std::uint8_t builder_topology(Dword *, Dword);
int find_builder_parameter(Dword *, const char *);
bool select_builder_parameters(Dword *, const char *);
void bind_builder_parameter_index(Dword *, int, void *, int, int);
void bind_builder_parameter(Dword *, const char *, void *, int, int);
std::uint8_t begin_model_builder(Dword *);
Dword *construct(Dword *);
Dword *construct_textured(Dword *);
void destroy(Dword *);
void destroy_textured(Dword *);
Dword initialize(Dword *);
Dword initialize_textured(Dword *);
void flush(Dword *);
void flush_textured(Dword *);
void set_primitive(Dword *, Dword);
void end_primitive(Dword *);
void set_texture(Dword *, Dword);
void set_textured_primitive(Dword *, Dword);
void end_textured_primitive(Dword *);
void textured_vertex(Dword *, const float *, const Dword *, const float *);
void vertex(Dword *, const float *, const Dword *);
void set_gouraud_state(Dword *, const Dword *);
void set_render_state(Dword *, const Dword *);
} // namespace nfs::graphics::immediate
namespace nfs::graphics::bindings {
extern Dword *triangle_texture_alpha;
extern Dword *triangle_texture;
extern Dword *triangle_alpha;
extern Dword *triangle;
extern Dword *line_alpha;
extern Dword *line;
extern Dword *single_draw;
extern Dword *single_draw_texture;
extern Dword previous_depth_method;
extern Dword enabled;
extern Dword matrix[16];
void initialize();
void shutdown();
} // namespace nfs::graphics::bindings
