#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_instruments.hpp"
#include "nfs/core/frame/game_step.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::game::world::camera {
// Preserve the mode's upper bytes as well as the AL predicate.
kernel::Dword hud_visible(void *director) {
    const kernel::Dword mode = top_mode(director);
    const bool visible = !game::runtime::at<std::uint8_t>(director, 172) && mode <= 3 &&
        !game::runtime::at<std::uint8_t>(director, 164) && !game::runtime::at<std::uint8_t>(director, 165) &&
        !game::runtime::at<std::uint8_t>(director, 166);
    return (mode & 4294967040u) | kernel::Dword(visible);
}
}
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;

void __fastcall initialize_view(View *view, void *) {
    gui::widgets::pending::request_general_layout(&view->base, nullptr);
    gui::events::propagate(view, 104, 0, 0, 2);
}

void __fastcall view_timer(View *view, void *, Dword timer) {
    if (timer == view->timer && !frame_clock::paused)
        gui::events::propagate(view, 104, 0, 0, 0);
}

void __fastcall tick_view(View *view, void *, Dword) {
    if (!network::packets::rule_value(system(72), 47)) {
        pending::set_widget_visible(view, 0);
        return;
    }
    void *server = system(108);
    if (at<Dword>(server, 44) && at<Dword>(server, 48)) {
        pending::set_widget_visible(view, 0);
        return;
    }
    const int player = view->second_player != 0;
    if (!game::runtime::physics::local_car(system(68), player)) {
        pending::set_widget_visible(view, 0);
        return;
    }
    void *camera = game::tick_services[player];
    if (camera && !Byte(game::world::camera::hud_visible(camera))) {
        pending::set_widget_visible(view, 0);
        return;
    }
    void *race = at<void *>(system(92), 4);
    void *car = game::runtime::physics::local_car(system(68), player);
    pending::set_widget_visible(view, !pending::participant_blocked(race, at<Dword>(car, 1772)));
}

void *view_camera(View *view) { return game::tick_services[view->second_player != 0]; }

void *view_car(View *view) {
    return game::runtime::physics::local_car(system(68), view->second_player != 0);
}

Dword __fastcall load_view(View *view, void *, void *configuration) {
    gui::engine::pending::read_int(configuration, "HudView.mbIsPlayer2", &view->second_player, 0);
    return gui::widgets::pending::load_general(&view->base, nullptr, configuration);
}

Dword __fastcall save_view(View *view, void *, void *configuration) {
    gui::engine::pending::write_int(configuration, "HudView.mbIsPlayer2", view->second_player, 0);
    return gui::widgets::pending::save_general(&view->base, nullptr, configuration);
}

void __fastcall draw_view(View *view, void *, void *engine) {
    invoke<void>(engine, 14, inherited_font_style(&view->base.base));
}

View *construct_view(View *view, void *parent) {
    construct_widget(view, parent);
    view->second_player = 0;
    gui::engine::construct_point(&view->position);
    try {
        view->base.base.methods = view_methods.data();
        view->base.base.flags = (view->base.base.flags & ~Dword(4)) | 1;
        view->timer = gui::create_widget_timer(view->base.base.id, 0.066666670143604278564453125f);
    } catch (...) {
        gui::engine::destroy_point(&view->position);
        destroy_general(&view->base);
        throw;
    }
    return view;
}

static void destroy_view(View *view) {
    view->base.base.methods = view_methods.data();
    try { gui::remove_widget_timer(view->timer); }
    catch (...) {
        gui::engine::destroy_point(&view->position);
        destroy_general(&view->base);
        throw;
    }
    gui::engine::destroy_point(&view->position);
    destroy_general(&view->base);
}

static View *__fastcall delete_view(View *view, void *, Dword flags) {
    destroy_view(view);
    if (flags & 1) gui::release(view);
    return view;
}

const char *__cdecl view_class_key() { return "HudView"; }

static const char *__fastcall view_name(View *, void *) { return "HudView"; }

static Dword __fastcall view_is_type(View *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(view_class_key) ||
        type == reinterpret_cast<const void *>(general_class_name) ||
        type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall view_is_name(View *, void *, const char *name) {
    return !std::strncmp(name, "HudView", 8) || !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> view_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_view), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::view_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::view_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::view_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::load_view), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::save_view), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::draw_view), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::initialize_view), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::view_timer), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tick_view), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
};

void *create_view(void *parent) {
    auto *view = static_cast<View *>(gui::allocate_flags(sizeof(View), 0, "HudView"));
    if (!view) return nullptr;
    try { return construct_view(view, parent); }
    catch (...) { gui::release(view); throw; }
}
}
