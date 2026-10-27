#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/resources/files/filesystem.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace nfs::graphics::backend {

static void select_native_context(Dword *render_context) {
    graphics::current_context = render_context;
}
 
void pump_window_messages(Dword *render_context) {
    selected_device();
    select_native_context(render_context);
    selected_device();
    while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
        // Let the owning application loop receive the thread's quit request.
        if (message.message == WM_QUIT) {
            PostQuitMessage(static_cast<int>(message.wParam));
            break;
        }
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}
} // namespace nfs::graphics::backend
namespace nfs::gui::engine {
using kernel::numeric::Extended;
Rectangle *to_pixels(Rectangle *, const Rectangle *);

Dword rectangles_differ(const Rectangle *first, const Rectangle *second) {
    return first->left != second->left || first->top != second->top ||
           first->height != second->height || first->width != second->width;
}

Dword *copy_colour(Dword *out, const Dword *colour) {
    *out = *colour;
    return out;
}
namespace pending {
using game::runtime::at;
using game::runtime::invoke;
Rectangle *intersect_rectangle(Rectangle *, const Rectangle *, const Rectangle *);
void load_font_styles(Engine *);
void load_palettes(Engine *);

void write_stats_exit_marker() {
    char filename[511];
    std::sprintf(filename, "%s\\stats.ini", graphics::startup::configuration_path());
    const char marker = ' ';
    Dword file;
    files::open_sync(filename, 2, 100, &file);
    files::write_sync(file, 0, &marker, 1, 100);
    files::close_sync(file, 100);
}

void __fastcall begin_frame(Engine *engine, void *) {
    Dword *context = engine->context;
    if (!context)
        return;
    if (lifecycle::level_load_failed) {
        write_stats_exit_marker();
        std::exit(0);
    }
    const auto callback = engine->frame_callback;
    if (callback)
        callback();
    float width, height;
    graphics::backend::get_dimensions(context, &width, &height);
    // FILD compares the complete signed dimensions; TEST AH,44h accepts unordered too.
    if (double(destination_bounds.width) != double(width) ||
        double(destination_bounds.height) != double(height)) {
        const auto format = kernel::numeric::current_format();
        const int pixel_height = int(Extended(height, format).truncate_to_integer());
        const int pixel_width = int(Extended(width, format).truncate_to_integer());
        Rectangle pixels{rectangle_methods, 0, 0, pixel_width, pixel_height};
        configure_coordinates(reinterpret_cast<const Rectangle *>(
                                  game::runtime::bytes(game::runtime::system(96)) + 84),
                              &pixels);
        input::mouse_bounds(0, 0, 0, pixel_width, pixel_height, 255);
    }
    invoke(engine, 308 / 4);
    invoke(engine, 268 / 4);
    engine->frame_bounds.left = default_bounds.left;
    engine->frame_bounds.top = default_bounds.top;
    engine->frame_bounds.height = default_bounds.height;
    engine->frame_bounds.width = default_bounds.width;
    graphics::viewports::rectangle(engine->viewport, float(destination_bounds.left),
                                   float(destination_bounds.top), float(destination_bounds.width),
                                   float(destination_bounds.height), 0, 1);
    Dword result, colour;
    Dword *returned = invoke<Dword *>(engine, 312 / 4, &result);
    copy_colour(&colour, returned);
    Dword *viewport = engine->viewport;
    graphics::viewports::set_color(viewport, colour);
    graphics::backend::pump_window_messages(engine->context);
    graphics::viewports::begin(viewport);
    if (engine->clear_frame)
        graphics::viewports::clear(viewport, 3);
}

Dword update_scissor(Engine *engine) {
    const Rectangle *clip = invoke<const Rectangle *>(engine, 264 / 4);
    Rectangle intersection, pixels;
    intersect_rectangle(&intersection, clip, &source_bounds);
    to_pixels(&pixels, &intersection);
    if (rectangles_differ(&pixels, &engine->frame_bounds)) {
        if (pixels.width && pixels.height) {
            graphics::viewports::end(engine->viewport);
            graphics::viewports::rectangle(engine->viewport, float(pixels.left), float(pixels.top),
                                           float(pixels.width), float(pixels.height), 0, 1);
            graphics::viewports::begin(engine->viewport);
        }
        engine->frame_bounds.top = pixels.top;
        engine->frame_bounds.left = pixels.left;
        engine->frame_bounds.width = pixels.width;
        engine->frame_bounds.height = pixels.height;
    }
    return pixels.width && pixels.height;
}

void __fastcall end_frame(Engine *engine, void *) {
    if (!engine->context)
        return;
    if (engine->images.count)
        if (void *image = events::container_value(&engine->images, 0)) {
            Dword colour = 16777215;
            Rectangle rectangle{rectangle_methods, -5, -5, 1, 1};
            invoke(engine, 180 / 4, at<Dword>(image, 4), &rectangle, &colour, Dword(0));
        }
    update_scissor(engine);
    invoke(engine, 300 / 4);
    invoke(engine, 268 / 4);
    graphics::viewports::end(engine->viewport);
    graphics::backend::present(engine->context);
    invoke(engine, 200 / 4, Dword(1));
}

void __fastcall reset_resources(Engine *engine, void *) {
    container_clear(&engine->font_styles);
    container_clear(&engine->palettes);
    container_clear(&engine->images);
    container_clear(&engine->textures);
    engine->palette = &engine->default_palette;
    engine->font = &engine->default_font;
    load_font_styles(engine);
    load_palettes(engine);
}
// The cursor-name construction is present even though its value is unused.
void clear_cursor_images(Engine *engine) {
    if (!engine->context)
        return;
    String name;
    string_construct(&name);
    try {
        char suffix[256];
        for (unsigned row = 0; row < 16; ++row)
            for (unsigned column = 0; column < 10; ++column) {
                Dword &image = reinterpret_cast<Dword *>(engine->vertex_scratch)[row * 10 + column];
                if (image)
                    invoke(engine, 120 / 4, image);
                std::sprintf(suffix, "c%02d%d", row, column);
                string_assign(&name, "[+Cursor] ");
                string_append_ascii(&name, suffix);
            }
    } catch (...) {
        string_destruct(&name);
        throw;
    }
    string_destruct(&name);
}

void __fastcall clear_resources(Engine *engine, void *) {
    container_clear(&engine->images);
    container_clear(&engine->textures);
    clear_cursor_images(engine);
}
} // namespace pending
} // namespace nfs::gui::engine
