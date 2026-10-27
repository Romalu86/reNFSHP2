#include "../widgets/gui_widget_shared_methods.hpp"
#include "hud_display.hpp"
#include <cstring>
namespace nfs::application::frontend::hud {
using namespace gui::widgets;
static Text *__fastcall delete_text(Text *, void *, Dword);

static Text *__fastcall delete_text_binding(Binding *binding, void *, Dword flags) {
    return delete_text(reinterpret_cast<Text *>(reinterpret_cast<Byte *>(binding) - 312), nullptr,
                       flags);
}
static const Method text_binding_methods[1] = {reinterpret_cast<Method>(delete_text_binding)};

static Text *construct_text(Text *widget, void *parent) {
    construct_flash_text(&widget->base, parent);
    widget->binding.owner = reinterpret_cast<Widget *>(widget);
    widget->binding.methods = binding_methods;
    reinterpret_cast<Widget *>(widget)->methods = text_methods.data();
    widget->binding.methods = text_binding_methods;
    reinterpret_cast<Widget *>(widget)->flags =
        (reinterpret_cast<Widget *>(widget)->flags & ~Dword(4)) | 1;
    return widget;
}

static void destroy_text(Text *widget) {
    reinterpret_cast<Widget *>(widget)->methods = text_methods.data();
    widget->binding.methods = text_binding_methods;
    widget->binding.methods = binding_methods;
    destroy_flash_text(&widget->base);
}

static Text *__fastcall delete_text(Text *widget, void *, Dword flags) {
    destroy_text(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const char *__cdecl text_class_key() {
    return "HudText";
}

static const char *__fastcall text_name(Text *, void *) {
    return "HudText";
}

static Dword __fastcall text_is_type(Text *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(text_class_key) ||
           type == reinterpret_cast<const void *>(flash_text_class_key) ||
           type == reinterpret_cast<const void *>(family::text_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall text_is_name(Text *, void *, const char *name) {
    return !std::strncmp(name, "HudText", 8) || !std::strncmp(name, "FlashText", 10) ||
           !std::strncmp(name, "GText", 6) || !std::strncmp(name, "GWidget", 8) ||
           !std::strncmp(name, "GUI", 4);
}
static const gui::engine::pending::FlagDefinition text_display_types[] = {
    {"HT_STATIC", 0, 0},        {"HT_SPEED", 0, 1},         {"HT_SPEEDRAW", 0, 2},
    {"HT_GEAR", 0, 3},          {"HT_TACH", 0, 4},          {"HT_POSITION", 0, 5},
    {"HT_OF_POSITION", 0, 6},   {"HT_POINTS", 0, 7},        {"HT_STATS_1", 0, 8},
    {"HT_STATS_2", 0, 9},       {"HT_STATS_3", 0, 10},      {"HT_STATSLABEL_1", 0, 11},
    {"HT_STATSLABEL_2", 0, 12}, {"HT_STATSLABEL_3", 0, 13}, {"HT_IND_ROADBLOCKS", 0, 14},
    {"HT_IND_HELI", 0, 15},     {"HT_IND_BACKUP", 0, 16},   {nullptr, 0, 0}};

static Dword __fastcall load_text(Text *widget, void *, void *configuration) {
    gui::engine::pending::read_enum(configuration, "HudText.mDisplayType", &widget->display_type,
                                    text_display_types, 0);
    return load_flash_text(&widget->base, nullptr, configuration);
}

static Dword __fastcall save_text(Text *widget, void *, void *configuration) {
    gui::engine::pending::write_enum(configuration, "HudText.mDisplayType", widget->display_type,
                                     text_display_types, 0);
    return save_flash_text(&widget->base, nullptr, configuration);
}

const std::array<Method, 49> text_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::hud::delete_text), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::text_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::text_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::text_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::load_text), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::save_text), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::text_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_flash_text), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::initialize_text), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::application::frontend::flash_text_timer), // 23.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::family::draw_text_line), // 47.
    reinterpret_cast<Method>(::nfs::application::frontend::hud::begin_text), // 48.
};

void *create_text(void *parent) {
    auto *widget = static_cast<Text *>(gui::allocate_flags(sizeof(Text), 0, "HudText"));
    if (!widget)
        return nullptr;
    try {
        return construct_text(widget, parent);
    } catch (...) {
        gui::release(widget);
        throw;
    }
}
} // namespace nfs::application::frontend::hud
