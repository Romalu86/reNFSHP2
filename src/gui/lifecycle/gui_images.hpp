#pragma once
#include "nfs/gui/engine/gui_engine.hpp"
namespace nfs::gui::engine::images {
struct Texture {
    const Destroy *methods;
    Dword *file, *raster, *atlas;
    int frame_count, format;
    Dword references, handle;
    String directory, name;
};
struct Image {
    const Method *methods;
    Dword handle, references;
    String name;
    Dword texture;
};
struct ShapeImage {
    Image base;
    Dword depth;
    Dword *raster;
    Rectangle bounds;
    Point origin;
};
struct AtlasImage {
    Image base;
    Dword *raster;
    Rectangle bounds, atlas_bounds;
    Point origin;
};
struct TexturePoint {
    const Destroy *methods;
    float x, y;
};
static_assert(sizeof(Texture) == 72 && offsetof(Texture, directory) == 32);
static_assert(sizeof(Image) == 36 && sizeof(ShapeImage) == 76 && sizeof(AtlasImage) == 92);
extern const Destroy texture_methods[];
extern const Method base_methods[8], shape_methods[8], atlas_methods[8];
Engine *resource_engine();
Texture *construct_texture(Texture *, const char *, const char *, const char *);
void destroy_texture(Texture *);
Texture *find_texture(Engine *, const String *);
Image *find_image(Engine *, Dword, const String *);
Image *frame(Texture *, const String *, Dword);
Image *frame_at(Texture *, int, Dword);
Dword rebind(Texture *, Image *, int);
ShapeImage *construct_shape(ShapeImage *, Dword *, const String *, Dword);
AtlasImage *construct_atlas(AtlasImage *, Dword *, Dword *, const String *, int, int, int, int, Dword);
void destroy_shape(ShapeImage *);
void destroy_atlas(AtlasImage *);
void destroy_image(Image *);
void release_texture_binding(Image *);
Byte suspend_drawing();
int find_separator(const String *, const char *);
void trim_left(String *, int);
void trim_right(String *, int);
String *frame_name(Texture *, String *, int);
} // namespace nfs::gui::engine::images
