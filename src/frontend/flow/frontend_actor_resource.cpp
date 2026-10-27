#include "frontend_scene_widgets.hpp"
#include "../../police/arrest/arrest_cinematic_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::actor_resources {
void bind_actor_texture(void *, void *);

void load_frontend_actor(void *actor, const char *name, Dword animation, ActorVector position,
                         ActorVector facing, const char *offset_kind) {
    char filename[264];
    std::sprintf(filename, "%sActorDef\\%s.adf", bytes(subsystem(4)) + 784, name);
    if (!files::file::exists(filename))
        return;
    ini::Reader reader;
    ini::reader_construct(&reader, filename, 0);
    try {
        vehicle_render::allocate_animation_channels(offset(actor, 52), "fe anim channel", 3,
                                                    lifecycle::character_pool);
        int model_number = 0, texture_number = 0;
        char resource[32], model[32], texture[32];
        ini::get_int(&reader, "actor", "modelNumber", &model_number);
        ini::get_int(&reader, "actor", "textureNumber", &texture_number);
        ini::get_string(&reader, "actor", "actorName", resource, 31);
        std::sprintf(model, "Model%d", model_number);
        std::sprintf(texture, "Tex%d.fsh", texture_number);
        if (void *child = at<void *>(actor, 264))
            yaw_actor(child, 90.f);
        float rotation[16], direction[3];
        actor_axis_rotation(rotation, 90.f, 0, 1, 0);
        std::memcpy(direction, offset(actor, 136), 12);
        graphics::math::transform_position(direction, rotation, direction);
        runtime::normalize_vector(direction, direction);
        std::memcpy(offset(actor, 136), &facing, sizeof facing);
        at<float>(actor, 176) = 50;
        at<Dword>(actor, 172) = 0;
        std::sprintf(reinterpret_cast<char *>(offset(actor, 416)), "actor%d", 0);
        std::memcpy(offset(actor, 124), &position, sizeof position);
        at<float>(actor, 72) = 1;
        bind_actor_resources(actor, resource, model, texture, 1, 0, 0);
        at<Dword>(actor, 268) = *offset_kind == 'a' || *offset_kind == 'A' ? 2 : 1;
        vehicle_render::pending::select_character_animation(actor, animation, 0);
        at<Dword>(actor, 376) = 1042983550;
        if (void *child = at<void *>(actor, 264))
            yaw_actor(child, 90.f);
        actor_axis_rotation(rotation, 90.f, 0, 1, 0);
        std::memcpy(direction, offset(actor, 136), 12);
        graphics::math::transform_position(direction, rotation, direction);
        runtime::normalize_vector(direction, direction);
        std::memcpy(offset(actor, 136), direction, 12);
        reset_animation_clock(offset(actor, 52));
        runtime::physics::clear_ground_contact(offset(actor, 180));
        at<Byte>(actor, 373) = 1;
        at<Byte>(actor, 84) = 1;
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void refresh_actor_texture(void *actor) {
    void *model = at<void *>(actor, 392), *texture = at<void *>(actor, 396);
    if (model && texture)
        bind_actor_texture(model, texture);
}
} // namespace nfs::game::world::actor_resources
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::at;
using kernel::numeric::Extended;

ActorPreview *construct_actor_preview(ActorPreview *widget, void *parent) {
    construct_scene_widget(&widget->base, parent);
    widget->actor = nullptr;
    widget->extent = 0;
    try {
        gui::string_construct_copy(&widget->resource,
                                   &lifecycle::construction::unresolved::shared_string);
    } catch (...) {
        destroy_scene_widget(&widget->base);
        throw;
    }
    widget->animation = 10101;
    widget->frame = widget->frame_step = 0;
    widget->loop = 0;
    try {
        gui::string_construct_copy(&widget->offset_type,
                                   &lifecycle::construction::unresolved::shared_string);
    } catch (...) {
        gui::string_destruct(&widget->resource);
        destroy_scene_widget(&widget->base);
        throw;
    }
    widget->base.base.base.methods = actor_preview_methods.data();
    return widget;
}

void destroy_actor_preview(ActorPreview *widget) {
    widget->base.base.base.methods = actor_preview_methods.data();
    if (widget->actor) {
        game::world::vehicle_render::pending::release_character(game::runtime::system(48),
                                                                widget->actor);
        widget->actor = nullptr;
    }
    gui::string_destruct(&widget->offset_type);
    gui::string_destruct(&widget->resource);
    destroy_scene_widget(&widget->base);
}

void load_preview_actor(ActorPreview *widget) {
    if (!widget->resource.length)
        return;
    widget->actor =
        game::world::actor_resources::create_actor(game::runtime::system(48), "fe Actor");
    float direction[] = {1, 0, 0},
          position[] = {widget->base.position.x, widget->base.position.y, widget->base.position.z};
    game::runtime::matrices::transform_vectors(1, direction, widget->base.rotation, direction);
    game::world::actor_resources::load_frontend_actor(widget->actor,
                                                      gui::string_ascii(&widget->resource),
                                                      widget->animation,
                                                      {position[0], position[1], position[2]},
                                                      {direction[0], direction[1], direction[2]}, "D");
    game::world::actor_resources::refresh_actor_texture(widget->actor);
    if (widget->base.pitch != 0 || std::isnan(widget->base.pitch))
        at<Byte>(widget->actor, 372) = 1;
}
// Consumes the resource name passed by value.
void set_preview_actor(ActorPreview *widget, gui::String name) {
    struct Cleanup {
        gui::String *value;
        ~Cleanup() {
            gui::string_destruct(value);
        }
    } cleanup{&name};
    gui::string_copy(&widget->resource, &name);
    load_preview_actor(widget);
}

void release_preview_actor(ActorPreview *widget) {
    if (widget->actor) {
        game::world::vehicle_render::pending::release_character(game::runtime::system(48),
                                                                widget->actor);
        widget->actor = nullptr;
    }
}

void __fastcall initialize_actor_preview(ActorPreview *widget, void *) {
    initialize_scene_widget(&widget->base, nullptr);
    widget->frame_step = 1;
    load_preview_actor(widget);
}

void __fastcall tick_actor_preview(ActorPreview *widget, void *, Dword timer) {
    tick_scene_widget(&widget->base, nullptr, timer);
    if (timer != widget->base.rotation_timer)
        return;
    const auto format = kernel::numeric::current_format();
    const float frame = float(Extended(widget->frame_step, format) + widget->frame);
    widget->frame = frame;
    if (!widget->actor)
        return;
    if (!(frame > 0)) {
        if (frame < 0)
            widget->frame = 0;
    } else {
        const float maximum = at<float>(at<void *>(widget->actor, 64), 12);
        if (!(frame < maximum) || std::isnan(frame) || std::isnan(maximum))
            widget->frame = maximum;
    }
}

Dword __fastcall load_actor_preview(ActorPreview *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_int(configuration, "ActorWidget.mAnimation", reinterpret_cast<int *>(&widget->animation),
             0);
    int loop = 0;
    read_int(configuration, "ActorWidget.mbLoopAnim", &loop, 0);
    widget->loop = loop != 0;
    read_reference_string(configuration, "ActorWidget.mOffsetType", &widget->offset_type,
                          &lifecycle::construction::unresolved::shared_string);
    return load_scene_widget(&widget->base, nullptr, configuration);
}

Dword __fastcall save_actor_preview(ActorPreview *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_int(configuration, "ActorWidget.mAnimation", int(widget->animation), 0);
    write_int(configuration, "ActorWidget.mbLoopAnim", widget->loop, 0);
    write_reference_string(configuration, "ActorWidget.mOffsetType", &widget->offset_type,
                           &lifecycle::construction::unresolved::shared_string);
    return save_scene_widget(&widget->base, nullptr, configuration);
}
} // namespace nfs::application::frontend
