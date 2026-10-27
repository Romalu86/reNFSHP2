#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "gui_images.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "gui_render_numeric_x87.hpp"
namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::gui::engine::images {
using game::runtime::at;
using game::runtime::invoke;
static Dword image_serial = 1;
static void construct_image(Image *image, const String *name, Dword texture) {
    image->methods = reinterpret_cast<const Method *>(gui::base_methods);
    kernel::lifecycle::Cleanup base_cleanup(
        [=] { image->methods = reinterpret_cast<const Method *>(gui::base_methods); });
    image->handle = image_serial;
    image_serial += 4;
    image->references = 1;
    string_construct_copy(&image->name, name);
    kernel::lifecycle::Cleanup name_cleanup([=] { string_destruct(&image->name); });
    image->texture = texture;
    image->methods = base_methods;
    if (texture)
        pending::acquire_texture(resource_engine(), nullptr, texture);
    name_cleanup.dismiss();
    base_cleanup.dismiss();
}

void release_texture_binding(Image *image) {
    if (image->texture) {
        pending::release_texture(resource_engine(), nullptr, image->texture);
        image->texture = 0;
    }
    if (image->texture) {
        pending::release_texture(resource_engine(), nullptr, image->texture);
        image->texture = 0;
    }
}

void destroy_image(Image *image) {
    image->methods = base_methods;
    kernel::lifecycle::Cleanup base_cleanup(
        [=] { image->methods = reinterpret_cast<const Method *>(gui::base_methods); });
    kernel::lifecycle::Cleanup name_cleanup([=] { string_destruct(&image->name); });
    release_texture_binding(image);
}

ShapeImage *construct_shape(ShapeImage *image, Dword *shape, const String *name, Dword texture) {
    construct_image(&image->base, name, texture);
    kernel::lifecycle::Cleanup image_cleanup([=] { destroy_image(&image->base); });
    image->depth = 0;
    image->raster = nullptr;
    construct_rectangle(&image->bounds);
    kernel::lifecycle::Cleanup bounds_cleanup([=] { destroy_rectangle(&image->bounds); });
    construct_point(&image->origin);
    kernel::lifecycle::Cleanup origin_cleanup([=] { destroy_coordinate(&image->origin); });
    image->base.methods = shape_methods;
    if (shape) {
        const int width = at<short>(shape, 4), height = at<short>(shape, 6);
        const bool width_valid = graphics::textures::is_power(Dword(width)) != 0;
        if (!width_valid || !graphics::textures::is_power(Dword(height))) {
            String message;
            string_construct(&message);
            try {
                // Height diagnostic supplies one value for three specifiers; its remaining vararg stack values
                // are outside accepted behavior.
                string_format_ascii(&message,
                                    !width_valid ? "ERROR: Bad shape width for %s\n"
                                                 : "ERROR: Bad shape height for [%s:%s] %s\n",
                                    string_ascii(name));
            } catch (...) {
                string_destruct(&message);
                throw;
            }
            string_destruct(&message);
        } else {
            const int packed = int(shape[3]);
            image->bounds.top = int(Dword(packed) << 4) >> 20;
            image->bounds.left = int(Dword(packed) << 20) >> 20;
            image->bounds.width = width;
            image->bounds.height = height;
            image->origin.x = at<short>(shape, 8);
            image->origin.y = at<short>(shape, 10);
            image->depth = graphics::tar::image_depth(shape);
            auto *raster =
                static_cast<Dword *>(lifecycle::allocate(48, "feTex", lifecycle::ui_pool));
            kernel::lifecycle::Cleanup raster_cleanup([=] {
                game::runtime::track::pending::release_named(raster, "feTex",
                                                                      lifecycle::ui_pool);
            });
            Dword *constructed = raster ? graphics::tar::construct_shape(raster, shape) : nullptr;
            raster_cleanup.dismiss();
            image->raster = constructed;
            if (image->raster)
                image->raster[1] = image->raster[2] = image->raster[3] = image->raster[4] = 1;
        }
    }
    origin_cleanup.dismiss();
    bounds_cleanup.dismiss();
    image_cleanup.dismiss();
    return image;
}

AtlasImage *construct_atlas(AtlasImage *image, Dword *shape, Dword *raster, const String *name, int x,
                            int y, int width, int height, Dword texture) {
    construct_image(&image->base, name, texture);
    kernel::lifecycle::Cleanup image_cleanup([=] { destroy_image(&image->base); });
    image->raster = raster;
    image->bounds = Rectangle{rectangle_methods, y, x, width, height};
    kernel::lifecycle::Cleanup bounds_cleanup([=] { destroy_rectangle(&image->bounds); });
    construct_rectangle(&image->atlas_bounds);
    kernel::lifecycle::Cleanup atlas_bounds_cleanup(
        [=] { destroy_rectangle(&image->atlas_bounds); });
    construct_point(&image->origin);
    kernel::lifecycle::Cleanup origin_cleanup([=] { destroy_coordinate(&image->origin); });
    image->base.methods = atlas_methods;
    if (raster)
        raster[1] = raster[2] = raster[3] = raster[4] = 1;
    if (shape) {
        const int packed = int(shape[3]);
        image->atlas_bounds.top = int(Dword(packed) << 4) >> 20;
        image->atlas_bounds.left = int(Dword(packed) << 20) >> 20;
        image->atlas_bounds.width = at<short>(shape, 4);
        image->atlas_bounds.height = at<short>(shape, 6);
        image->origin.x = at<short>(shape, 8);
        image->origin.y = at<short>(shape, 10);
    }
    origin_cleanup.dismiss();
    atlas_bounds_cleanup.dismiss();
    bounds_cleanup.dismiss();
    image_cleanup.dismiss();
    return image;
}

static void __fastcall release_shape(ShapeImage *image, void *) {
    if (image->raster) {
        graphics::tar::destroy(image->raster);
        eagl::deallocator(image->raster, 48);
        image->raster = nullptr;
    }
    image->depth = 0;
    release_texture_binding(&image->base);
}

static void __fastcall release_atlas(AtlasImage *image, void *) {
    image->raster = nullptr;
    release_texture_binding(&image->base);
}

void destroy_shape(ShapeImage *image) {
    image->base.methods = shape_methods;
    kernel::lifecycle::Cleanup image_cleanup([=] { destroy_image(&image->base); });
    kernel::lifecycle::Cleanup bounds_cleanup([=] { destroy_rectangle(&image->bounds); });
    kernel::lifecycle::Cleanup origin_cleanup([=] { destroy_coordinate(&image->origin); });
    release_shape(image, nullptr);
}

void destroy_atlas(AtlasImage *image) {
    image->base.methods = atlas_methods;
    kernel::lifecycle::Cleanup image_cleanup([=] { destroy_image(&image->base); });
    kernel::lifecycle::Cleanup bounds_cleanup([=] { destroy_rectangle(&image->bounds); });
    kernel::lifecycle::Cleanup atlas_bounds_cleanup(
        [=] { destroy_rectangle(&image->atlas_bounds); });
    kernel::lifecycle::Cleanup origin_cleanup([=] { destroy_coordinate(&image->origin); });
    release_atlas(image, nullptr);
}

static Image *__fastcall delete_image(Image *image, void *, Dword flags) {
    destroy_image(image);
    if (flags & 1)
        gui::release(image);
    return image;
}

static ShapeImage *__fastcall delete_shape(ShapeImage *image, void *, Dword flags) {
    destroy_shape(image);
    if (flags & 1)
        gui::release(image);
    return image;
}

static AtlasImage *__fastcall delete_atlas(AtlasImage *image, void *, Dword flags) {
    destroy_atlas(image);
    if (flags & 1)
        gui::release(image);
    return image;
}

static Dword __fastcall rebind_base(Image *image, void *, Dword *, Dword *, int, int, int, int,
                                   Dword texture) {
    if (texture != image->texture)
        invoke(image, 7);
    image->texture = texture;
    string_copy(&image->name, &lifecycle::construction::unresolved::shared_string);
    return 1;
}

static Byte __fastcall rebind_shape(ShapeImage *image, void *, Dword *shape, Dword *, int, int, int,
                                    int, Dword texture) {
    if (!shape)
        return 1;
    const int width = at<short>(shape, 4), height = at<short>(shape, 6);
    if (!graphics::textures::is_power(Dword(width)) || !graphics::textures::is_power(Dword(height)))
        return 0;
    if (texture != image->base.texture)
        invoke(image, 7);
    image->base.texture = texture;
    string_copy(&image->base.name, &lifecycle::construction::unresolved::shared_string);
    const Dword depth = graphics::tar::image_depth(shape);
    if (image->raster &&
        (width != image->bounds.width || height != image->bounds.height || depth != image->depth)) {
        graphics::tar::destroy(image->raster);
        eagl::deallocator(image->raster, 48);
        image->raster = nullptr;
    }
    const int packed = int(shape[3]);
    image->bounds.left = int(Dword(packed) << 20) >> 20;
    image->bounds.top = int(Dword(packed) << 4) >> 20;
    image->bounds.width = width;
    image->bounds.height = height;
    image->depth = depth;
    image->origin.x = at<short>(shape, 8);
    image->origin.y = at<short>(shape, 10);
    if (image->raster)
        return graphics::tar::replace_image(image->raster, shape);
    auto *raster = static_cast<Dword *>(lifecycle::allocate(48, "feTex", lifecycle::ui_pool));
    kernel::lifecycle::Cleanup raster_cleanup([=] {
        game::runtime::track::pending::release_named(raster, "feTex", lifecycle::ui_pool);
    });
    Dword *constructed = raster ? graphics::tar::construct_shape(raster, shape) : nullptr;
    raster_cleanup.dismiss();
    image->raster = constructed;
    if (constructed)
        constructed[1] = constructed[2] = constructed[3] = constructed[4] = 1;
    return 1;
}

static Dword __fastcall rebind_atlas(AtlasImage *image, void *, Dword *shape, Dword *raster, int x,
                                    int y, int width, int height, Dword texture) {
    if (!shape || !raster)
        return 0;
    if (texture != image->base.texture)
        invoke(image, 7);
    image->base.texture = texture;
    string_copy(&image->base.name, &lifecycle::construction::unresolved::shared_string);
    const int packed = int(shape[3]);
    image->atlas_bounds.left = int(Dword(packed) << 20) >> 20;
    image->atlas_bounds.top = int(Dword(packed) << 4) >> 20;
    image->atlas_bounds.width = at<short>(shape, 4);
    image->atlas_bounds.height = at<short>(shape, 6);
    image->origin.x = at<short>(shape, 8);
    image->origin.y = at<short>(shape, 10);
    image->bounds.left = x;
    image->bounds.top = y;
    image->bounds.height = height;
    image->bounds.width = width;
    image->raster = raster;
    return 1;
}

static Dword __fastcall shape_uv(ShapeImage *image, void *, TexturePoint *first,
                                TexturePoint *second) {
    if (!image->raster)
        return 0;

    const auto x = gui_numeric::number(0.5f) / gui_numeric::integer(image->bounds.width);
    const auto y = gui_numeric::number(0.5f) / gui_numeric::integer(image->bounds.height);
    first->x = float(x);
    first->y = float(y);
    second->x = float(gui_numeric::number(1.f) - x);
    second->y = float(gui_numeric::number(1.f) - y);
    return 1;
}

static Dword __fastcall atlas_uv(AtlasImage *image, void *, TexturePoint *first,
                                TexturePoint *second) {
    if (!image->raster)
        return 0;

    const auto width = gui_numeric::integer(gui_numeric::read_field(image->atlas_bounds.width));
    const int left = image->bounds.left, top = image->bounds.top;
    first->x = float((gui_numeric::integer(left) + 0.5f) / width);
    const auto height = gui_numeric::integer(gui_numeric::read_field(image->atlas_bounds.height));
    first->y =
        float((gui_numeric::integer(gui_numeric::read_field(image->bounds.top)) + 0.5f) / height);
    const int right = int(Dword(gui_numeric::read_field(image->bounds.width)) + Dword(left));
    const int bottom = int(Dword(gui_numeric::read_field(image->bounds.height)) + Dword(top));
    second->x = float((gui_numeric::integer(right) + 0.5f) / width);
    second->y = float((gui_numeric::integer(bottom) + 0.5f) / height);
    return 1;
}

static Dword __fastcall shape_bounds(ShapeImage *image, void *, Rectangle *bounds, Point *origin) {
    if (bounds) {
        bounds->left = image->bounds.left;
        bounds->top = image->bounds.top;
        bounds->height = image->bounds.height;
        bounds->width = image->bounds.width;
    }
    if (origin) {
        origin->x = image->origin.x;
        origin->y = image->origin.y;
    }
    return 1;
}

static Dword __fastcall atlas_bounds(AtlasImage *image, void *, Rectangle *bounds, Point *origin) {
    if (bounds) {
        bounds->left = image->bounds.left;
        bounds->top = image->bounds.top;
        bounds->height = image->bounds.height;
        bounds->width = image->bounds.width;
    }
    if (origin) {
        origin->x = image->origin.x;
        origin->y = image->origin.y;
    }
    return 1;
}

static Dword *__fastcall shape_raster(ShapeImage *image, void *) {
    return image->raster;
}

static Dword *__fastcall atlas_raster(AtlasImage *image, void *) {
    return image->raster;
}
namespace drawing {
void __fastcall shape(ShapeImage *, void *, const Point *, const Point *, const Point *,
                      const Point *, const Dword *, Dword *, Dword *);
void __fastcall shape_uv(ShapeImage *, void *, const Point *, const Point *, const Point *,
                         const Point *, const Dword *, const TexturePoint *, const TexturePoint *,
                         Dword *, Dword *);
void __fastcall atlas(AtlasImage *, void *, const Point *, const Point *, const Point *,
                      const Point *, const Dword *, Dword *, Dword *);
} // namespace drawing

static void __fastcall draw_atlas_uv(AtlasImage *image, void *, const Point *first,
                                     const Point *second, const Point *third, const Point *fourth,
                                     const Dword *colour, const TexturePoint *, const TexturePoint *,
                                     Dword *viewport, Dword *draw) {
    invoke(image, 1, first, second, third, fourth, colour, viewport, draw);
}
static void __fastcall release_base(Image *image, void *) {
    release_texture_binding(image);
}
#define METHOD(function) reinterpret_cast<Method>(function)
const Method base_methods[8] = {METHOD(delete_image),
                                METHOD(::nfs::assertions::pure_virtual_call),
                                METHOD(::nfs::assertions::pure_virtual_call),
                                METHOD(::nfs::assertions::pure_virtual_call),
                                METHOD(rebind_base),
                                METHOD(::nfs::assertions::pure_virtual_call),
                                METHOD(::nfs::assertions::pure_virtual_call),
                                METHOD(release_base)};
const Method shape_methods[8] = {METHOD(delete_shape),      METHOD(drawing::shape),
                                 METHOD(drawing::shape_uv), METHOD(shape_bounds),
                                 METHOD(rebind_shape),      METHOD(shape_raster),
                                 METHOD(shape_uv),          METHOD(release_shape)};
const Method atlas_methods[8] = {METHOD(delete_atlas),  METHOD(drawing::atlas),
                                 METHOD(draw_atlas_uv), METHOD(atlas_bounds),
                                 METHOD(rebind_atlas),  METHOD(atlas_raster),
                                 METHOD(atlas_uv),      METHOD(release_atlas)};
#undef METHOD
} // namespace nfs::gui::engine::images
