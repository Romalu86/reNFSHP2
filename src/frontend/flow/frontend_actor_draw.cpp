#include "frontend_scene_widgets.hpp"
#include "../../graphics/actors/character_runtime.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include <cstring>
namespace nfs::application::frontend {
using game::runtime::at;
using game::runtime::invoke;

void __fastcall draw_actor_preview(ActorPreview *widget, void *, gui::engine::Engine *engine) {
    invoke<void>(engine, 56, Dword(6), Dword(0), Dword(0));
    gui::engine::Rectangle rectangle = *gui::engine::current_viewport_bounds(&engine->base), pixels;
    gui::engine::to_pixels(&pixels, &rectangle);
    rectangle.top = pixels.top;
    rectangle.left = pixels.left;
    rectangle.width = pixels.width;
    rectangle.height = pixels.height;
    gui::engine::destroy_rectangle(&pixels);
    float x = 0, y = 0, width = 0, height = 0;
    Dword minimum = 0, maximum = 0;
    graphics::viewports::read_rectangle(graphics::main_viewport, &x, &y, &width, &height, &minimum,
                                        &maximum);
    graphics::viewports::rectangle(graphics::main_viewport, float(rectangle.left),
                                   float(rectangle.top), float(rectangle.width),
                                   float(rectangle.height), 0, 1);
    game::world::camera::apply_menu_camera(game::tick_services[0], graphics::main_viewport, 1, 0);
    if (void *actor = widget->actor) {
        std::memcpy(game::runtime::bytes(actor) + 336, widget->base.rotation, 36);
        graphics::viewports::begin(graphics::main_viewport);
        graphics::viewports::clear(graphics::main_viewport, 2);
        if (widget->base.rotates && widget->base.pitch == 0)
            game::world::actor_resources::yaw_actor(actor, widget->base.rotation_angle);
        if (at<std::uint8_t>(actor, 373))
            game::world::vehicle_render::restore_skeleton_pose(
                at<void *>(at<void *>(actor, 88), 24),
                game::world::vehicle_render::animation_pose_scratch(), nullptr);
        void *channels = at<void *>(actor, 64);
        if (at<float>(channels, 4) <= at<float>(channels, 12)) {
            Dword frame;
            std::memcpy(&frame, &widget->frame, 4);
            game::world::vehicle_render::pending::select_character_animation(
                actor, widget->animation, frame);
            at<Dword>(actor, 376) = 1051372191;
        }
        if (at<std::uint8_t>(actor, 373) && at<void *>(actor, 384)) {
            game::world::actor_resources::evaluate_actor_channel(
                game::runtime::bytes(actor) + 52, 0,
                game::world::vehicle_render::animation_pose_scratch());
            game::world::actor_resources::evaluate_actor_channel(
                game::runtime::bytes(actor) + 52, 1,
                game::world::vehicle_render::animation_pose_scratch());
        }
        if (widget->loop && at<float>(channels, 12) < at<float>(channels, 4)) {
            widget->frame = float(kernel::numeric::Extended(at<float>(channels, 4),
                                                            kernel::numeric::current_format()) -
                                  at<float>(channels, 12));
            Dword frame;
            std::memcpy(&frame, &widget->frame, 4);
            for (int channel = 0; channel < 3; ++channel)
                game::world::actor_resources::set_channel_offset(game::runtime::bytes(actor) + 52,
                                                                 channel, frame);
        }
        graphics::backend::set_depth_write(graphics::context, 0);
        invoke<void>(actor, 2, game::tick_services[0], graphics::main_viewport);
        graphics::backend::set_depth_write(graphics::context, 1);
        graphics::viewports::end(graphics::main_viewport);
    }
    float near_value, far_value;
    std::memcpy(&near_value, &minimum, 4);
    std::memcpy(&far_value, &maximum, 4);
    graphics::viewports::rectangle(graphics::main_viewport, x, y, width, height, near_value,
                                   far_value);
    invoke<void>(engine, 56, Dword(5), Dword(0), Dword(0));
    gui::engine::destroy_rectangle(&rectangle);
}
} // namespace nfs::application::frontend
