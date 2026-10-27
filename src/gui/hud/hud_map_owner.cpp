#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/core/lifecycle/scope_cleanup.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_map.hpp"
#include <cstring>
#include <cstdlib>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;

static Map *construct_map(Map *map, void *parent) {
    construct_widget(map, parent);
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_general(&map->base); });
    map->vertex_count = 0;
    map->half_width = map->half_height = map->centre_x = map->centre_z = 0;
    map->zoom = 3;
    for (float &sample : map->zoom_history)
        sample = 0;
    map->pursuit_zoom = 12;
    map->pursuit_distance = 0;
    map->zoom_rate = 6;
    for (float &sample : map->heading_history)
        sample = 0;
    map->pitch = 0;
    map->translation[1] = map->translation[2] = map->translation[0] = 0;
    map->style = 1;
    construct_image_resource(&map->car_shape);
    kernel::lifecycle::Cleanup car_cleanup([=] { destroy_image_resource(&map->car_shape); });
    construct_image_resource(&map->opponent_shape);
    kernel::lifecycle::Cleanup opponent_cleanup([=] { destroy_image_resource(&map->opponent_shape); });
    construct_image_resource(&map->perpetrator_shape);
    kernel::lifecycle::Cleanup perpetrator_cleanup([=] { destroy_image_resource(&map->perpetrator_shape); });
    construct_image_resource(&map->cop_shape);
    kernel::lifecycle::Cleanup cop_cleanup([=] { destroy_image_resource(&map->cop_shape); });
    map->background = map->spline_colour = 0;
    map->reversed = 0;
    map->geometry = nullptr;
    map->section_detail_limit = 1;
    map->first_draw = 1;
    map->pool = lifecycle::ui_pool | 1024;
    map->extent = 5;
    map->base.base.methods = map_methods.data();
    Dword *rules = static_cast<Dword *>(game::runtime::system(72));
    if (network::packets::rule_value(rules, 6) == 2 ||
        network::packets::rule_value(rules, 6) == 3)
        map->reversed = 1;
    map->base.base.flags = (map->base.base.flags & ~Dword(4)) | 1;
    cop_cleanup.dismiss();
    perpetrator_cleanup.dismiss();
    opponent_cleanup.dismiss();
    car_cleanup.dismiss();
    base_cleanup.dismiss();
    return map;
}

static void destroy_map(Map *map) {
    map->base.base.methods = map_methods.data();
    kernel::lifecycle::Cleanup base_cleanup([=] { destroy_general(&map->base); });
    kernel::lifecycle::Cleanup car_cleanup([=] { destroy_image_resource(&map->car_shape); });
    kernel::lifecycle::Cleanup opponent_cleanup([=] { destroy_image_resource(&map->opponent_shape); });
    kernel::lifecycle::Cleanup perpetrator_cleanup([=] { destroy_image_resource(&map->perpetrator_shape); });
    destroy_image_resource(&map->cop_shape);
}

static Map *__fastcall delete_map(Map *map, void *, Dword flags) {
    destroy_map(map);
    if (flags & 1)
        gui::release(map);
    return map;
}

const char *__cdecl map_class_key() {
    return "HudMap";
}

static const char *__fastcall map_name(Map *, void *) {
    return "HudMap";
}

static Dword __fastcall map_is_type(Map *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(map_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall map_is_name(Map *, void *, const char *name) {
    return !std::strncmp(name, "HudMap", 7) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}

const std::array<Method, 47> map_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_map), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::map_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::map_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::map_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::load_map), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::save_map), // 5.
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
    reinterpret_cast<Method>(::nfs::application::frontend::hud::prepare_map), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::draw_map), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::initialize_map), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::tick_map), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::release_map), // 25.
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

void *create_map(void *parent) {
    auto *map = static_cast<Map *>(gui::allocate_flags(sizeof(Map), 0, "HudMap"));
    if (!map)
        return nullptr;
    try {
        return construct_map(map, parent);
    } catch (...) {
        gui::release(map);
        throw;
    }
}
static ClassRecord map_class;

static void unregister_map() {
    unregister_class(&map_class);
}

static void register_map() {
    register_class(&map_class, "HudMap", create_map, "GWidget");
    std::atexit(unregister_map);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_map, ".CRT$XCU_NFS_006900B0",
                       register_map);
} // namespace
} // namespace nfs::application::frontend::hud
