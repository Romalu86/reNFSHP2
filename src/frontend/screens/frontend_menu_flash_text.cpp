#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../flow/frontend_flash_text.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
static const gui::engine::pending::FlagDefinition flash_styles[] = {
    {"FTS_FLASHONBEGIN", 0, 1}, {"FTS_FLASHHIDEWHENDONE", 0, 2}, {nullptr, 0, 0}};

const char *__cdecl flash_text_class_key() {
    return "FlashText";
}

static const char *__fastcall flash_text_name(Widget *, void *) {
    return "FlashText";
}

static Dword __fastcall flash_text_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(flash_text_class_key) ||
           type == reinterpret_cast<const void *>(family::text_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall flash_text_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "FlashText", 10) || !std::strncmp(name, "GText", 6) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

FlashText *construct_flash_text(FlashText *widget, void *parent) {
    gui::widgets::pending::construct_text(widget, parent);
    widget->timer = 0;
    widget->progress = 0;
    widget->active = 0;
    widget->style = 0;
    widget->speed = widget->grow_y = 1;
    widget->grow_x = 3;
    widget->destination_colour = 0;
    widget->base.base.methods = flash_text_methods.data();
    try {
        widget->timer = gui::create_widget_timer(widget->base.base.id, 0.05f);
    } catch (...) {
        family::text_destroy(&widget->base.base);
        throw;
    }
    return widget;
}

void destroy_flash_text(FlashText *widget) {
    widget->base.base.methods = flash_text_methods.data();
    gui::remove_widget_timer(widget->timer);
    family::text_destroy(&widget->base.base);
}

static FlashText *__fastcall delete_flash_text(FlashText *widget, void *, Dword flags) {
    destroy_flash_text(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

Dword __fastcall load_flash_text(FlashText *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    read_float(configuration, "FlashText.mSpeed", &widget->speed, 0);
    read_float(configuration, "FlashText.mGrowX", &widget->grow_x, 0);
    read_float(configuration, "FlashText.mGrowY", &widget->grow_y, 0);
    const Dword fallback = 0;
    read_colour(configuration, "FlashText.mDestColor", &widget->destination_colour, &fallback);
    read_flags(configuration, "FlashText.mFlashStyle", &widget->style, flash_styles, 0);
    return family::text_load(&widget->base.base, nullptr, reinterpret_cast<Dword>(configuration));
}

Dword __fastcall save_flash_text(FlashText *widget, void *, void *configuration) {
    using namespace gui::engine::pending;
    write_float(configuration, "FlashText.mSpeed", widget->speed, 2, 0);
    write_float(configuration, "FlashText.mGrowX", widget->grow_x, 2, 0);
    write_float(configuration, "FlashText.mGrowY", widget->grow_y, 2, 0);
    const Dword fallback = 0;
    write_colour(configuration, "FlashText.mDestColor", &widget->destination_colour, &fallback);
    write_flags(configuration, "FlashText.mFlashStyle", widget->style, flash_styles, 0);
    return family::text_save(&widget->base.base, nullptr, reinterpret_cast<Dword>(configuration));
}

void __fastcall begin_flash_text(FlashText *widget, void *) {
    if (!widget->active) {
        widget->active = 1;
        widget->progress = 0;
    }
}

void __fastcall initialize_flash_text(FlashText *widget, void *) {
    gui::widgets::pending::request_general_layout(&widget->base, nullptr);
    if (widget->style & 1)
        invoke(widget, 48);
}

Dword __fastcall flash_text_event(FlashText *widget, void *, Dword event, Dword first,
                                        Dword second) {
    return gui::widgets::pending::general_event(&widget->base, nullptr, event, first, second);
}


const std::array<Method, 49> flash_text_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_flash_text), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::flash_text_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::flash_text_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::flash_text_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_flash_text), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_flash_text), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::text_layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::flash_text_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::application::frontend::draw_flash_text), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_flash_text), // 21.
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
    reinterpret_cast<Method>(::nfs::application::frontend::begin_flash_text), // 48.
};

static void *create_flash_text(void *parent) {
    auto *storage =
        static_cast<FlashText *>(gui::allocate_flags(sizeof(FlashText), 0, "FlashText"));
    if (!storage)
        return nullptr;
    try {
        return construct_flash_text(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord flash_text_class;

static void unregister_flash_text() {
    unregister_class(&flash_text_class);
}

static void register_flash_text() {
    register_class(&flash_text_class, "FlashText", create_flash_text, "GText");
    std::atexit(unregister_flash_text);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_flash_text, ".CRT$XCU_NFS_00690088",
                       register_flash_text);
} // namespace
} // namespace nfs::application::frontend
