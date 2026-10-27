#include "hud_display.hpp"
#include "../../police/pursuit/chase_runtime.hpp"
#include "../../network/session/network_pursuit_runtime.hpp"
#include "../widgets/gui_widget_interaction.hpp"
#include <cstdio>
namespace nfs::application::frontend::hud {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
using namespace gui::widgets;
using kernel::numeric::Extended;

static Widget *registered_widget(Dword id) {
    return id ? static_cast<Widget *>(gui::events::find_widget(gui::context_state, id)) : nullptr;
}

void *car_chase_state(void *car) {
    void *entity = at<void *>(car, 7988);
    if (entity) {
        auto *descriptor = at<const int *>(entity, 4);
        void *perpetrator = game::runtime::bytes(entity) + 4 + descriptor[2];
        void *owner = invoke<void *>(perpetrator, 3);
        if (void *chase = at<void *>(owner, 8))
            return game::runtime::bytes(chase) + 52;
    }
    return nullptr;
}

void construct_progress_pair(ProgressPair *pair) {
    pair->first = pair->second = pair->active = pair->inactive = nullptr;
    pair->selected = 0;
    pair->blinking = pair->hidden = 0;
}

void toggle_progress_pair(ProgressPair *pair) {
    if (pair && pair->blinking) {
        pair->hidden = !pair->hidden;
        pending::set_widget_visible(pair->active, !pair->hidden);
    }
}

void select_progress_pair(ProgressPair *pair, Byte blinking, float fraction) {
    pair->blinking = blinking;
    if (!blinking) {
        pair->selected = 0;
        pair->active = pair->second;
        pair->inactive = pair->first;
        pending::set_widget_visible(pair->second, 1);
    } else {
        pair->selected = 1;
        pair->active = pair->first;
        pair->inactive = pair->second;
    }
    ProgressBar *active = pair->active, *inactive = pair->inactive;
    active->fraction = fraction;
    inactive->fraction = fraction;
    pending::set_widget_visible(inactive, 0);
    set_enabled(reinterpret_cast<Widget *>(active), 1);
    pending::set_widget_visible(active, 1);
    set_enabled(reinterpret_cast<Widget *>(inactive), 0);
    pending::set_widget_visible(inactive, 0);
}

void __fastcall progress_bar_timer(ProgressBar *widget, void *, Dword timer) {
    if (timer == widget->timer)
        toggle_progress_pair(widget->pair);
}

void __fastcall draw_progress_bar(ProgressBar *widget, void *, gui::engine::Engine *engine) {
    if (!(widget->fraction > 0))
        return;
    gui::engine::Rectangle bounds = *gui::engine::current_viewport_bounds(&engine->base);
    const auto format = kernel::numeric::current_format();
    bounds.width = int(
        Dword((Extended::integer(bounds.width, format) * widget->fraction).truncate_to_integer()));
    if (bounds.width > 0) {
        invoke(engine, 64, &bounds);
        family::static_image_draw(reinterpret_cast<Widget *>(widget), nullptr,
                                  reinterpret_cast<Dword>(engine));
        invoke(engine, 65);
    }
}

void __fastcall initialize_cuffometer(Cuffometer *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    gui::engine::top_left(&widget->position, &widget->base.base.bounds);
    for (int index = 0; index < 5; ++index) {
        char name[80];
        std::sprintf(name, "<star%d>", index);
        auto *star = static_cast<Widget *>(pending::find_child(widget, name));
        if (star && !invoke<Dword>(star, 2, family::static_image_class_key))
            star = nullptr;
        widget->stars[index] = reinterpret_cast<StaticImage *>(star);
        if (star)
            pending::set_widget_visible(star, 0);
    }
    auto child = [&](const char *name) {
        auto *bar = static_cast<Widget *>(pending::find_child(widget, name));
        return bar && invoke<Dword>(bar, 2, progress_bar_class_key)
                   ? reinterpret_cast<ProgressBar *>(bar)
                   : nullptr;
    };
    auto *first = child("<bar>"), *second = child("<bar2>");
    if (first) {
        auto *base = reinterpret_cast<Widget *>(first);
        base->value_84 = base->colour;
        first->pair = &widget->progress;
        widget->progress.second = first;
    }
    if (second) {
        auto *base = reinterpret_cast<Widget *>(second);
        base->value_84 = base->colour;
        widget->progress.first = second;
    }
}

void update_cuffometer(Cuffometer *widget) {
    void *car = binding_car(&widget->binding);
    if (!car) {
        pending::set_widget_visible(widget, 0);
        widget->chase_state = 0;
        return;
    }
    const Dword mode = at<Dword>(system(72), 152);
    void *chase = mode != 10 && mode != 4 ? car_chase_state(car) : nullptr;
    if (!chase) {
        pending::set_widget_visible(widget, 0);
        widget->chase_state = 0;
        widget->progress.blinking = 0;
        return;
    }
    widget->chase_state = at<Dword>(chase, 12);
    const Dword value = at<Dword>(chase, 20);
    const auto age = std::int32_t(graphics::unresolved::camera_state -
                                  game::runtime::ai::lifecycle::pending::threat_memory_ticks);
    const Byte blinking = value < at<Dword>(chase, 32) && age <= at<int>(chase, 28);
    int stars = at<int>(chase, 0);
    if (mode == 6 && stars)
        stars = 6;
    const auto format = kernel::numeric::current_format();
    const float fraction = float(Extended::integer(at<int>(chase, 20), format) /
                                 Extended::integer(at<int>(chase, 16), format));
    select_progress_pair(&widget->progress, blinking, fraction);
    for (int index = 0; index < 5; ++index)
        if (widget->stars[index])
            pending::set_widget_visible(widget->stars[index], index < stars);
    if (at<Byte>(car, 1816)) {
        auto *reset = static_cast<Widget *>(pending::find_child(widget, "Reset"));
        if (reset && invoke<Dword>(reset, 2, flash_text_class_key)) {
            pending::set_widget_visible(reset, 1);
            invoke(reset, 48);
            Dword player = 0;
            if (at<Dword>(system(72), 176) == 2)
                player = car != game::runtime::physics::local_car(system(68), 0);
            game::runtime::ai::cars::play_unavailable_action(player);
            at<Byte>(car, 1816) = 0;
        }
    }
    pending::set_widget_visible(widget, 1);
}

void __fastcall prepare_cuffometer(Cuffometer *widget, void *, gui::engine::Engine *engine) {
    prepare_draw(&widget->base.base, nullptr, engine);
    auto *mirror = registered_widget(rear_view_id);
    if (mirror) {
        widget->base.base.bounds.left = widget->position.x;
        widget->base.base.bounds.top = widget->position.y;
        if (mirror->flags & 2) {
            gui::engine::Point zero{gui::engine::point_methods, 0, 0}, screen, relative;
            family::to_screen_point(mirror, &screen, &zero);
            family::from_screen_point(&widget->base.base, &relative, &screen);
            widget->base.base.bounds.left = widget->position.x;
            widget->base.base.bounds.top = int(Dword(relative.y) + Dword(widget->position.y));
        }
    }
}

Dword __fastcall cuffometer_event(Cuffometer *widget, void *, Dword event, Dword first, Dword second) {
    if (event == 104)
        update_cuffometer(widget);
    return gui::widgets::pending::general_event(&widget->base, nullptr, event, first, second);
}
} // namespace nfs::application::frontend::hud
