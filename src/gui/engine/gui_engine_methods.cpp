#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/timing/timer.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../widgets/gui_widget_resources.hpp"
#include <cmath>
namespace nfs::gui::engine {
using game::runtime::at;
using game::runtime::invoke;
Dword caret_visible{}, caret_deadline{};

static Base *__fastcall delete_base(Base *engine, void *, unsigned flags) {
    destroy_base(engine);
    if (flags & 1)
        gui::release(engine);
    return engine;
}

static Engine *__fastcall delete_engine(Engine *engine, void *, unsigned flags) {
    destroy(engine);
    if (flags & 1)
        gui::release(engine);
    return engine;
}

static void __fastcall draw_root(Base *engine, void *, void *root) {
    pending::draw_root(root, engine, 1);
}

static void __fastcall push_clip(Base *engine, void *, const Rectangle *rectangle) {
    if (engine->clip_count < 32) {
        auto &clip = engine->clips[engine->clip_count];
        clip.left = rectangle->left;
        clip.top = rectangle->top;
        clip.height = rectangle->height;
        clip.width = rectangle->width;
        ++engine->clip_count;
    }
}

static void __fastcall pop_clip(Base *engine, void *) {
    if (engine->clip_count)
        --engine->clip_count;
}
// The saved method returns a reference, never an owned rectangle.
static const Rectangle *__fastcall current_clip(Base *engine, void *) {
    return engine->clip_count ? &engine->clips[engine->clip_count - 1] : &source_bounds;
}

static void __fastcall push_viewport(Base *engine, void *, const Rectangle *rectangle) {
    if (engine->viewport_count < 32) {
        auto &viewport = engine->viewports[engine->viewport_count];
        viewport.left = rectangle->left;
        viewport.top = rectangle->top;
        viewport.height = rectangle->height;
        viewport.width = rectangle->width;
        ++engine->viewport_count;
    }
}

static void __fastcall pop_viewport(Base *engine, void *) {
    if (engine->viewport_count)
        --engine->viewport_count;
}
namespace pending {
Rectangle *intersect_rectangle(Rectangle *, const Rectangle *, const Rectangle *);
}

static Dword __fastcall clipping_empty(Base *engine, void *) {
    const Rectangle *viewport =
        engine->viewport_count ? &engine->viewports[engine->viewport_count - 1] : &source_bounds;
    const Rectangle *clip = invoke<const Rectangle *>(engine, 264 / 4);
    Rectangle intersection;
    pending::intersect_rectangle(&intersection, clip, viewport);
    return !intersection.width || !intersection.height;
}

static void __fastcall draw_overlay(Base *engine, void *) {
    pending::draw_overlay(engine);
}

static void __fastcall visit_palettes(Engine *engine, void *,
                                      void(__cdecl *visitor)(void *, Dword, Dword, Dword), Dword first,
                                      Dword second) {
    for (Dword index = 0; index < engine->palettes.count; ++index)
        if (auto *palette =
                static_cast<Palette *>(events::container_value(&engine->palettes, index)))
            visitor(&palette->name, palette->handle, first, second);
}
// Complete entry is RET16.
static void __fastcall reserved_palette_operation(Engine *, void *, Dword, Dword, Dword, Dword) {
}
// Complete entry is RET12.
static void __fastcall reserved_font_operation(Engine *, void *, Dword, Dword, Dword) {
}
// Complete entry is RET12.
static void __fastcall reserved_texture_operation(Engine *, void *, Dword, Dword, Dword) {
}

static String *__fastcall default_font_style_name(Engine *, void *, String *out, Dword, Dword, Dword,
                                                  Dword) {
    gui::string_construct(out);
    gui::string_copy(out, &lifecycle::construction::unresolved::shared_string);
    return out;
}

static void __fastcall set_palette_colour(Engine *engine, void *, Dword handle, int index,
                                          const Dword *colour) {
    const Dword count = engine->palettes.count;
    for (Dword slot = 0; slot < count; ++slot)
        if (auto *palette =
                static_cast<Palette *>(events::container_value(&engine->palettes, slot));
            palette && palette->handle == handle) {
            palette->colours[index] = *colour;
            return;
        }
}

static Dword *__fastcall palette_colour(Engine *engine, void *, Dword *out, int index) {
    *out = engine->palette->colours[index];
    return out;
}
// Complete saved XOR EAX,EAX; RET8.
static Dword __fastcall reserved_image_operation(Engine *, void *, Dword, Dword) {
    return 0;
}
// Complete saved XOR EAX,EAX; RET12.
static Dword __fastcall reserved_texture_query(Engine *, void *, Dword, Dword, Dword) {
    return 0;
}
// FLDZ; RET8.
static long double __fastcall texture_time(Engine *, void *, Dword, Dword) {
    return 0;
}

static void __fastcall draw_uniform_rectangle(Engine *engine, void *, Dword rectangle, Dword colour,
                                              Dword flags) {
    invoke(engine, 188 / 4, rectangle, colour, colour, colour, colour, flags);
}

static Dword __fastcall raster_inset(Engine *engine, void *, Dword) {
    return engine->raster_inset;
}

static Dword __fastcall set_drawing_state(Engine *engine, void *, Dword value) {
    const Dword previous = engine->drawing_state;
    engine->drawing_state = value;
    return previous;
}
// Both coordinates use inverse horizontal scale.
static Point *__fastcall cursor_size(Engine *, void *, Point *out, Dword) {
    const float y = inverse_horizontal_scale * 32;
    const int rounded_y = int(std::lrintf(y));
    const float x = inverse_horizontal_scale * 32;
    out->methods = widgets::family::vector_methods;
    out->x = int(std::lrintf(x));
    out->y = rounded_y;
    return out;
}

static Dword __fastcall drawing_state(Engine *engine, void *) {
    return engine->drawing_state;
}

static void __fastcall change_drawing_depth(Engine *engine, void *, Dword entering) {
    engine->drawing_depth += entering ? 1 : Dword(-1);
}
// Complete saved XOR EAX,EAX; RET.
static Dword __fastcall reserved_render_query(Engine *, void *) {
    return 0;
}
// Complete RET8.
static void __fastcall reserved_render_operation(Engine *, void *, Dword, Dword) {
}

static Dword __fastcall image_format(Engine *engine, void *) {
    return engine->image_format;
}

int seconds_to_ticks(float seconds) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    const float ticks = float(Extended::integer(int(kernel::timer_frequency), format) *
                              Extended(seconds, format));
    return int(std::lrintf(ticks));
}

static Dword __fastcall caret_state(Engine *, void *) {
    const int now = int(kernel::timer_count());
    const float seconds = float(kernel::numeric::Extended::integer(
                                    GetCaretBlinkTime(), kernel::numeric::current_format()) *
                                0.001000000047497451305389404296875f);
    const int period = seconds_to_ticks(seconds);
    if (!period)
        return 0;
    if (int(caret_deadline) <= now) {
        caret_visible ^= 1;
        caret_deadline = Dword(now) + Dword(period) * (caret_visible ? 1 : 2);
    }
    return caret_visible;
}

static void __fastcall reset_caret(Engine *, void *) {
    caret_visible = 1;
    caret_deadline = 0;
}
// Complete saved XOR EAX,EAX; RET.
static Dword __fastcall reserved_engine_query(Engine *, void *) {
    return 0;
}

static void __fastcall reset_engine(Engine *engine, void *) {
    invoke(engine, 288 / 4);
}
// Complete RET.
static void __fastcall reserved_frame_operation(Engine *, void *) {
}

static Dword *__fastcall background_colour(Engine *, void *, Dword *out) {
    *out = at<Byte>(game::runtime::system(52), 81296) ? 4278190080u : 4282010938u;
    return out;
}
// The first61 base entries are the shipped purecall, not successful placeholders.
#define METHOD(function) reinterpret_cast<Method>(function)
const Method base_methods[73] = {METHOD(delete_base),        METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(::nfs::assertions::pure_virtual_call),
                                 METHOD(::nfs::assertions::pure_virtual_call),       METHOD(gui::bind_engine),
                                 METHOD(gui::unbind_engine), METHOD(draw_root),
                                 METHOD(push_clip),          METHOD(pop_clip),
                                 METHOD(current_clip),       METHOD(gui::reset_engine_queue_counts),
                                 METHOD(push_viewport),      METHOD(pop_viewport),
                                 METHOD(clipping_empty),     METHOD(reserved_engine_query),
                                 METHOD(draw_overlay)};
const Method methods[79] = {METHOD(delete_engine),
                            METHOD(visit_palettes),
                            METHOD(pending::visit_font_styles),
                            METHOD(pending::visit_fonts),
                            METHOD(reserved_palette_operation),
                            METHOD(reserved_font_operation),
                            METHOD(reserved_texture_operation),
                            METHOD(pending::find_font_style),
                            METHOD(pending::font_style_name),
                            METHOD(pending::create_font_style),
                            METHOD(default_font_style_name),
                            METHOD(pending::query_font_style),
                            METHOD(pending::update_font_style),
                            METHOD(pending::save_font_style),
                            METHOD(pending::select_font_style),
                            METHOD(pending::measure_string),
                            METHOD(pending::glyph_metrics),
                            METHOD(pending::font_pixel_height),
                            METHOD(pending::copy_palette),
                            METHOD(pending::find_palette),
                            METHOD(pending::palette_name),
                            METHOD(pending::select_palette),
                            METHOD(set_palette_colour),
                            METHOD(palette_colour),
                            METHOD(pending::save_palette),
                            METHOD(reserved_image_operation),
                            METHOD(pending::load_image),
                            METHOD(pending::acquire_image),
                            METHOD(pending::image_name),
                            METHOD(pending::query_image),
                            METHOD(pending::release_image),
                            METHOD(pending::load_texture),
                            METHOD(pending::acquire_texture),
                            METHOD(reserved_texture_query),
                            METHOD(pending::release_texture),
                            METHOD(pending::texture_frame_count),
                            METHOD(pending::texture_frame),
                            METHOD(pending::bind_image_frame),
                            METHOD(pending::find_texture_frame),
                            METHOD(pending::image_uses_texture),
                            METHOD(pending::texture_name),
                            METHOD(texture_time),
                            METHOD(pending::draw_string),
                            METHOD(pending::draw_image_quad),
                            METHOD(pending::draw_image_uv),
                            METHOD(pending::draw_image_rectangle),
                            METHOD(pending::draw_line),
                            METHOD(pending::draw_rectangle),
                            METHOD(draw_uniform_rectangle),
                            METHOD(raster_inset),
                            METHOD(set_drawing_state),
                            METHOD(cursor_size),
                            METHOD(drawing_state),
                            METHOD(change_drawing_depth),
                            METHOD(reserved_render_query),
                            METHOD(pending::clear_resources),
                            METHOD(pending::control_rendering),
                            METHOD(reserved_render_operation),
                            METHOD(image_format),
                            METHOD(caret_state),
                            METHOD(reset_caret),
                            METHOD(gui::bind_engine),
                            METHOD(gui::unbind_engine),
                            METHOD(pending::tick_root),
                            METHOD(push_clip),
                            METHOD(pop_clip),
                            METHOD(current_clip),
                            METHOD(gui::reset_engine_queue_counts),
                            METHOD(push_viewport),
                            METHOD(pop_viewport),
                            METHOD(clipping_empty),
                            METHOD(reserved_engine_query),
                            METHOD(draw_overlay),
                            METHOD(pending::reset_resources),
                            METHOD(pending::begin_frame),
                            METHOD(reset_engine),
                            METHOD(pending::end_frame),
                            METHOD(reserved_frame_operation),
                            METHOD(background_colour)};
#undef METHOD
} // namespace nfs::gui::engine
