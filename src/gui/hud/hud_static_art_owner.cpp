#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_display.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static StaticArt *__fastcall delete_static_art(StaticArt *, void *, Dword);

static StaticArt *__fastcall delete_static_art_binding(Binding *binding, void *, Dword flags) {
    return delete_static_art(
        reinterpret_cast<StaticArt *>(reinterpret_cast<Byte *>(binding) - 324), nullptr, flags);
}
static const Method static_art_binding_methods[1] = {
    reinterpret_cast<Method>(delete_static_art_binding)};

static StaticArt *construct_static_art(StaticArt *widget, void *parent) {
    gui::widgets::pending::construct_static_image(widget, parent);
    widget->binding.owner = reinterpret_cast<Widget *>(widget);
    widget->binding.methods = binding_methods;
    reinterpret_cast<Widget *>(widget)->methods = static_art_methods.data();
    widget->binding.methods = static_art_binding_methods;
    reinterpret_cast<Widget *>(widget)->flags =
        (reinterpret_cast<Widget *>(widget)->flags & ~Dword(4)) | 1;
    return widget;
}

static void destroy_static_art(StaticArt *widget) {
    reinterpret_cast<Widget *>(widget)->methods = static_art_methods.data();
    widget->binding.methods = static_art_binding_methods;
    widget->binding.methods = binding_methods;
    family::static_image_destroy(reinterpret_cast<Widget *>(widget));
}

static StaticArt *__fastcall delete_static_art(StaticArt *widget, void *, Dword flags) {
    destroy_static_art(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const char *__cdecl static_art_class_key() {
    return "HudStaticArt";
}

static const char *__fastcall static_art_name(StaticArt *, void *) {
    return "HudStaticArt";
}

static Dword __fastcall static_art_is_type(StaticArt *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(static_art_class_key) ||
           type == reinterpret_cast<const void *>(family::static_image_class_key) ||
           type == reinterpret_cast<const void *>(family::image_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall static_art_is_name(StaticArt *, void *, const char *name) {
    return !std::strncmp(name, "HudStaticArt", 13) || !std::strncmp(name, "GStaticImage", 13) ||
           !std::strncmp(name, "GImage", 7) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}
static const gui::engine::pending::FlagDefinition art_display_types[] = {
    {"HSA_STATIC", 0, 0},   {"HSA_SHIFTNOW", 0, 1},   {"HSA_IND_ROADBLOCKS", 0, 2},
    {"HSA_IND_HELI", 0, 3}, {"HSA_IND_BACKUP", 0, 4}, {"HSA_METRIC", 0, 5},
    {"HSA_IMPERIAL", 0, 6}, {nullptr, 0, 0}};

static Dword __fastcall load_static_art(StaticArt *widget, void *, void *configuration) {
    gui::engine::pending::read_enum(configuration, "HudStaticArt.mDisplayType",
                                    &widget->display_type, art_display_types, 0);
    return family::static_image_load(reinterpret_cast<Widget *>(widget), nullptr,
                                     reinterpret_cast<Dword>(configuration));
}

static Dword __fastcall save_static_art(StaticArt *widget, void *, void *configuration) {
    gui::engine::pending::write_enum(configuration, "HudStaticArt.mDisplayType",
                                     widget->display_type, art_display_types, 0);
    return family::static_image_save(reinterpret_cast<Widget *>(widget), nullptr,
                                     reinterpret_cast<Dword>(configuration));
}

const std::array<Method, 48> static_art_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_static_art), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::static_art_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::static_art_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::static_art_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::load_static_art), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::save_static_art), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::static_art_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_push_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_draw), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::request_general_layout), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::family::static_image_resource), // 47.
};

void *create_static_art(void *parent) {
    auto *widget =
        static_cast<StaticArt *>(gui::allocate_flags(sizeof(StaticArt), 0, "HudStaticArt"));
    if (!widget)
        return nullptr;
    try {
        return construct_static_art(widget, parent);
    } catch (...) {
        gui::release(widget);
        throw;
    }
}
} // namespace nfs::application::frontend::hud
