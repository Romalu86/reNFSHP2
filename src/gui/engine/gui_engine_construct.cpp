#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
#include "../lifecycle/gui_font_metrics.hpp"
#include <cmath>
namespace nfs::gui::engine {
static void destroy_members(Engine *engine, int completed = 13) {
    if (completed >= 13)
        graphics::immediate::destroy_textured(engine->textured_draw);
    if (completed >= 12)
        graphics::geoprim::base_destroy(engine->additive_state);
    if (completed >= 11)
        graphics::geoprim::base_destroy(engine->textured_state);
    if (completed >= 10)
        graphics::geoprim::base_destroy(engine->flat_state);
    if (completed >= 9)
        destroy_rectangle(&engine->resource_bounds);
    if (completed >= 8)
        gui::string_destruct(&engine->resource_name);
    if (completed >= 7)
        destroy_rectangle(&engine->frame_bounds);
    if (completed >= 6)
        gui::container_destruct(&engine->images);
    if (completed >= 5)
        gui::container_destruct(&engine->textures);
    if (completed >= 4)
        destroy_palette(&engine->default_palette);
    if (completed >= 3)
        gui::container_destruct(&engine->palettes);
    if (completed >= 2)
        gui::container_destruct(&engine->font_styles);
    if (completed >= 1)
        destroy_base(&engine->base);
}

Base *construct_base(Base *engine) {
    engine->methods = reinterpret_cast<const Method *>(gui::base_methods);
    bool clips_complete = false;
    try {
        lifecycle::construction::construct_array(engine->clips, sizeof(Rectangle), 32,
                                                 construct_rectangle, destroy_rectangle);
        clips_complete = true;
        lifecycle::construction::construct_array(engine->viewports, sizeof(Rectangle), 32,
                                                 construct_rectangle, destroy_rectangle);
        engine->methods = base_methods;
        engine->clip_count = engine->viewport_count = 0;
        return engine;
    } catch (...) {
        if (clips_complete)
            lifecycle::construction::objects::destroy_array(
                engine->clips, sizeof(Rectangle), 32, destroy_rectangle);
        engine->methods = reinterpret_cast<const Method *>(gui::base_methods);
        throw;
    }
}

void destroy_base(Base *engine) {
    engine->methods = base_methods;
    gui::unbind_engine(reinterpret_cast<Object *>(engine));
    bool clips_pending = true;
    try {
        lifecycle::construction::objects::destroy_array(
            engine->viewports, sizeof(Rectangle), 32, destroy_rectangle);
        clips_pending = false;
        lifecycle::construction::objects::destroy_array(
            engine->clips, sizeof(Rectangle), 32, destroy_rectangle);
        engine->methods = reinterpret_cast<const Method *>(gui::base_methods);
    } catch (...) {
        if (clips_pending)
            lifecycle::construction::objects::destroy_array(
                engine->clips, sizeof(Rectangle), 32, destroy_rectangle);
        engine->methods = reinterpret_cast<const Method *>(gui::base_methods);
        throw;
    }
}

Engine *construct(Engine *engine, Dword *context) {
    construct_base(&engine->base);
    int completed = 1;
    try {
        gui::container_construct(&engine->font_styles);
        completed = 2;
        gui::container_construct(&engine->palettes);
        completed = 3;
        engine->default_font.scale_x = engine->default_font.scale_y = 1;
        engine->default_font.flags = 0;
        engine->default_font.height = 0;
        engine->default_font.spacing = 0.7799999713897705078125f;
        engine->default_font.font = 0;
        engine->default_font.handle = 0;
        engine->font = nullptr;
        construct_palette(&engine->default_palette);
        completed = 4;
        engine->palette = nullptr;
        gui::container_construct(&engine->textures);
        completed = 5;
        engine->drawing_state = 0;
        engine->reserved_5b0 = 0;
        engine->drawing_depth = 0;
        gui::container_construct(&engine->images);
        completed = 6;
        engine->raster_inset = 0;
        engine->image_format = 0;
        engine->reserved_7b0 = 0;
        construct_rectangle(&engine->frame_bounds);
        completed = 7;
        gui::string_construct(&engine->resource_name);
        completed = 8;
        construct_rectangle(&engine->resource_bounds);
        completed = 9;
        engine->active_texture = 0;
        lifecycle::construction::render_state(engine->flat_state);
        completed = 10;
        lifecycle::construction::render_state(engine->textured_state);
        completed = 11;
        lifecycle::construction::render_state(engine->additive_state);
        completed = 12;
        graphics::immediate::construct_textured(engine->textured_draw);
        completed = 13;
        engine->context = context;
        engine->viewport = nullptr;
        engine->frame_callback = nullptr;
        engine->base.methods = methods;
        engine->clear_frame = 1;
        engine->scale_x = engine->scale_y = 1;
        String editor;
        gui::string_construct(&editor);
        try {
            gui::string_assign(&editor, "Editor");
            engine->default_font.font = find_font(&editor);
        } catch (...) {
            gui::string_destruct(&editor);
            throw;
        }
        gui::string_destruct(&editor);
        engine->active_texture = 0;
        initialize_font_metrics(engine, &engine->default_font);
        engine->default_palette.colours[0] = 4286545791u;
        engine->default_palette.colours[2] = 4286545791u;
        engine->default_palette.colours[1] = 4286545791u;
        engine->default_palette.colours[3] = 4286545791u;
        engine->default_palette.colours[5] = 4278190080u;
        engine->default_palette.colours[9] = 4278190080u;
        engine->default_palette.colours[8] = 4278190080u;
        engine->default_palette.colours[10] = 4278190080u;
        engine->font = &engine->default_font;
        engine->palette = &engine->default_palette;
        engine->default_palette.colours[4] = 4294967295u;
        engine->default_palette.colours[6] = 4293717228u;
        engine->default_palette.colours[7] = 4282335039u;
        engine->default_palette.colours[11] = 2139062271;
        engine->raster_inset = 1;
        kernel::memory_zero(engine->vertex_scratch, sizeof(engine->vertex_scratch));
        engine->reserved_5b0 = engine->image_format = 0;
        engine->drawing_state = engine->drawing_depth = 1;
        kernel::memory_zero(engine->image_scratch, sizeof(engine->image_scratch));
        for (unsigned index = 0; index < 16; ++index)
            engine->matrix[index] = index % 5 == 0 ? 1.0f : 0.0f;
        if (engine->context) {
            engine->viewport = graphics::create_viewport(engine->context);
            graphics::viewports::orthographic(engine->viewport, 1, 1, 1300);
            graphics::viewports::set_data(engine->viewport, 0);
            graphics::viewports::set_matrix(engine->viewport, engine->matrix);
            for (auto *state :
                 {engine->flat_state, engine->textured_state, engine->additive_state}) {
                graphics::geoprim::shading(state, 1);
                graphics::geoprim::texture_enable(state, state == engine->flat_state ? 0 : 1);
                graphics::geoprim::alpha_test(state, 0);
                graphics::geoprim::alpha_blend_mode(state, state == engine->additive_state ? 2 : 1);
                graphics::geoprim::transparency(state, 1);
                graphics::geoprim::culling(state, 0);
                graphics::geoprim::depth_method(state, 8);
            }
            kernel::memory_copy(graphics::bindings::single_draw + 84 / 4, engine->matrix,
                                sizeof(engine->matrix));
            graphics::immediate::initialize_textured(engine->textured_draw);
            kernel::memory_copy(reinterpret_cast<Byte *>(engine) + 3004, engine->matrix,
                                sizeof(engine->matrix));
        }
        engine->frame_callback = nullptr;
        return engine;
    } catch (...) {
        destroy_members(engine, completed);
        throw;
    }
}

void destroy(Engine *engine) {
    engine->base.methods = methods;
    if (engine->context && engine->viewport)
        graphics::destroy_viewport(engine->context, engine->viewport);
    destroy_members(engine);
}
} // namespace nfs::gui::engine
