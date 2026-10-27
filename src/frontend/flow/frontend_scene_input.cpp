#include "frontend_scene_widgets.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../../gui/lifecycle/gui_input.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "nfs/core/timing/timer.hpp"
#include <cmath>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using kernel::numeric::Extended;

void begin_scene_drag(SceneWidget *widget, const gui::engine::Point *point) {
    if (!widget->rotates)
        return;
    widget->drag_x = widget->previous_x = point->x;
    widget->drag_y = point->y;
    scene_euler_angles(widget->rotation, &widget->angles.x, &widget->angles.y, &widget->angles.z);
    widget->angles.y = -widget->angles.y;
    assign_vector_value(&widget->drag_angles, &widget->angles.x);
    widget->drag_velocity = 0;
    assign_vector_value(&widget->angular_step, game::runtime::saved::zero_vector);
    widget->drag_clock = 0;
}

void update_scene_drag(SceneWidget *widget, const gui::engine::Point *point) {
    if (!widget->rotates)
        return;
    const int start_x = widget->drag_x, start_y = widget->drag_y, x = point->x, y = point->y;
    const Dword now = kernel::timer_count();
    const auto format = kernel::numeric::current_format();
    if (!widget->drag_clock) {
        widget->drag_clock = now;
        widget->drag_velocity = 0;
    } else {
        const auto elapsed = Extended::integer(std::int32_t(now - widget->drag_clock), format);
        if (std::int32_t(now - widget->drag_clock) != 0) {
            const int previous = widget->previous_x;
            widget->drag_clock = now;
            widget->previous_x = x;
            widget->drag_velocity =
                float((Extended::integer(std::int32_t(Dword(previous) - Dword(x)), format) *
                       0.000833333353511989116668701171875f) / elapsed);
        }
    }
    scene_euler_angles(widget->rotation, &widget->angles.x, &widget->angles.y, &widget->angles.z);
    widget->angles.y = -widget->angles.y;
    auto zoom =
        Extended(float(int(Dword(start_y) - Dword(y))), format) * .1f + widget->drag_angles.x;
    widget->requested_zoom = float(zoom);
    if (zoom < Extended(-30.f, format))
        zoom = Extended(-30.f, format);
    else if (Extended(30.f, format) < zoom)
        zoom = Extended(30.f, format);
    widget->requested_zoom = float(zoom);
    game::runtime::matrices::rotate_x(widget->rotation, 0);
    scene_euler_angles(widget->rotation, &widget->angles.x, &widget->angles.y, &widget->angles.z);
    widget->angles.y = -widget->angles.y;
    const float heading =
        float(-(Extended(float(int(Dword(start_x) - Dword(x))), format) * 0.00124999997206032276153564453125f +
                widget->drag_angles.y));
    scene_euler_matrix(widget->angles.x, heading, widget->angles.z, widget->rotation);
}

void end_scene_drag(SceneWidget *widget) {
    if (!widget->rotates)
        return;
    float velocity = -widget->drag_velocity;
    if (velocity < -0.0055555556900799274444580078125f && !std::isnan(velocity))
        velocity = -0.0055555556900799274444580078125f;
    else if (velocity > 0.0055555556900799274444580078125f)
        velocity = 0.0055555556900799274444580078125f;
    widget->angular_step.y = velocity;
}

Dword __fastcall press_scene_widget(SceneWidget *widget, void *, Dword, Dword modifiers) {
    if (application::pending::captured_widget_id != widget->base.base.id &&
        !std::uint8_t(modifiers) && (modifiers & 4194304)) {
        capture_widget(&widget->base.base);
        begin_scene_drag(widget, &gui::input::pointer_coordinates);
    }
    return 0;
}

Dword __fastcall drag_scene_widget(SceneWidget *widget, void *, Dword first, Dword modifiers) {
    if (application::pending::captured_widget_id == widget->base.base.id &&
        (modifiers & 4194304)) {
        update_scene_drag(widget, &gui::input::pointer_coordinates);
        return 1;
    }
    return gui::widgets::pending::update_active_style(&widget->base.base, nullptr, first,
                                                      modifiers);
}

void __fastcall release_scene_pointer(SceneWidget *widget, void *, Dword first, Dword modifiers) {
    if (!(modifiers & 29360128)) {
        if (application::pending::captured_widget_id == widget->base.base.id) {
            end_scene_drag(widget);
            widget->requested_zoom = widget->zoom;
        }
        release_capture(&widget->base.base);
    }
    capture_style(&widget->base.base, nullptr, first, modifiers);
}

void __fastcall leave_scene_widget(SceneWidget *widget, void *, Dword argument) {
    notify_parent_enter(&widget->base.base, nullptr, argument);
    release_capture(&widget->base.base);
}

Dword __fastcall cursor_scene_widget(SceneWidget *widget, void *, void *engine) {
    if (!widget->rotates)
        return 0;
    const bool dragging = application::pending::captured_widget_id == widget->base.base.id &&
                          (gui::input::modifiers() & 4194304);
    invoke<void>(engine, 50, Dword(dragging ? 16 : 15));
    return 1;
}
} // namespace nfs::application::frontend
