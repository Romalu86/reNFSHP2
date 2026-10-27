#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../widgets/gui_widget_shared_methods.hpp"
#include "gui_ime.hpp"
#include "../../frontend/flow/frontend_runtime.hpp"
#include "nfs/graphics/fonts/font_faces.hpp"
#include "../../race/session/race_legacy_effects.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
namespace nfs::gui::widgets::ime {
using game::runtime::at;
using game::runtime::invoke;
using game::runtime::system;
namespace {
Widget *typed_child(void *owner, const char *name, TypeName type) {
    auto *child = static_cast<Widget *>(application::pending::find_child(owner, name));
    return child && invoke<Dword>(child, 2, type) ? child : nullptr;
}
Widget *callback_parent(void *sender) {
    auto *parent = static_cast<Widget *>(sender)->parent;
    return parent && invoke<Dword>(parent, 2, interface_class_name) ? parent : nullptr;
}
void set_label(Widget *button, const char *text) {
    application::pending::LocalFrontendString label(text);
    invoke(button, 7, &label.value);
}
} // namespace
// The atlas and viewport belong to the existing font/GUI services.
void draw_debug_atlas(Edit *) {
    void *frontend = system(96);
    auto *render = at<engine::Engine *>(frontend, 4);
    if (!render)
        render = at<engine::Engine *>(frontend, 8);
    if (render) {
        auto *manager = reinterpret_cast<fonts::faces::Manager *>(system(20));
        auto *atlas = manager->count > 4 ? manager->faces[4].patchwork : nullptr;
        if (atlas) {
            Dword colour;
            game::world::frame::pack_effect_colour(&colour, 255, 255, 255, 255);
            fonts::patchwork::draw_page(atlas, render->viewport, 200.0f, 140.0f, 1.0f, colour);
        }
    }
}

void __fastcall initialize_debug(DebugScreen *screen, void *) {
    screen->edit = reinterpret_cast<Edit *>(typed_child(screen, "<IME>", edit_class_key));
    ShowCursor(TRUE);
    Widget *button = typed_child(screen, "<EnableToggle>", family::button_class_key);
    if (screen->edit && button)
        set_label(button, context_disabled(screen->edit) ? "Enable" : "Disable");
    button = typed_child(screen, "<ActivateToggle>", family::button_class_key);
    if (screen->edit && button) {
        toggle_settings(screen->edit);
        set_label(button, context_open(screen->edit) ? "Deactivate" : "Activate");
    }
    pending::request_interface_layout(&screen->base, nullptr);
}

void __fastcall draw_debug(DebugScreen *screen, void *, Dword) {
    if (screen->edit)
        draw_debug_atlas(screen->edit);
}

Dword __cdecl settings_callback(void *sender, Dword, Dword, Dword) {
    if (auto *parent = callback_parent(sender))
        if (auto *edit = typed_child(parent, "<IME>", edit_class_key))
            toggle_settings(reinterpret_cast<Edit *>(edit));
    return 1;
}

Dword __cdecl information_callback(void *sender, Dword, Dword, Dword) {
    if (auto *parent = callback_parent(sender))
        if (auto *edit = typed_child(parent, "<IME>", edit_class_key))
            conversion_status(reinterpret_cast<Edit *>(edit));
    return 1;
}

Dword __cdecl enable_callback(void *sender, Dword, Dword, Dword) {
    if (auto *parent = callback_parent(sender)) {
        auto *edit = reinterpret_cast<Edit *>(typed_child(parent, "<IME>", edit_class_key));
        auto *button = typed_child(parent, "<EnableToggle>", family::button_class_key);
        if (edit) {
            if (context_disabled(edit)) {
                enable_context(edit);
                if (button)
                    set_label(button, "Disable");
            } else {
                detach_context(edit);
                if (button)
                    set_label(button, "Enable");
            }
        }
    }
    return 1;
}

Dword __cdecl activate_callback(void *sender, Dword, Dword, Dword) {
    if (auto *parent = callback_parent(sender)) {
        auto *edit = reinterpret_cast<Edit *>(typed_child(parent, "<IME>", edit_class_key));
        auto *button = typed_child(parent, "<ActivateToggle>", family::button_class_key);
        if (edit) {
            if (context_open(edit)) {
                close_context(edit);
                if (button)
                    set_label(button, "Deactivate");
            } else {
                open_context(edit);
                if (button)
                    set_label(button, "Activate");
            }
        }
    }
    return 1;
}

Dword __cdecl candidates_callback(void *sender, Dword, Dword, Dword) {
    if (auto *parent = callback_parent(sender)) {
        auto *edit = reinterpret_cast<Edit *>(typed_child(parent, "<IME>", edit_class_key));
        if (edit) {
            auto *list = typed_child(parent, "<IMECandidates>", candidate_class_key);
            if (list) {
                edit->candidate_id = list->id;
                refresh_candidates(edit);
            }
        }
    }
    return 1;
}

static DebugScreen *construct_debug(DebugScreen *screen, void *parent) {
    pending::construct_interface(screen, parent);
    screen->edit = nullptr;
    screen->base.methods = debug_methods.data();
    return screen;
}

static void destroy_debug(DebugScreen *screen) {
    screen->base.methods = debug_methods.data();
    destroy_interface(&screen->base);
}

static DebugScreen *__fastcall delete_debug(DebugScreen *screen, void *, Dword flags) {
    destroy_debug(screen);
    if (flags & 1)
        release(screen);
    return screen;
}

static const char *__fastcall debug_name(DebugScreen *, void *) {
    return "GIMEDebugScreen";
}

static const char *__cdecl debug_class_key() {
    return "GIMEDebugScreen";
}

static Dword __fastcall debug_is_name(DebugScreen *, void *, const char *name) {
    return !std::strncmp(name, "GIMEDebugScreen", 16) || !std::strncmp(name, "GInterface", 11) ||
           !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall debug_is_type(DebugScreen *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(debug_class_key) ||
           type == reinterpret_cast<const void *>(interface_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

const std::array<Method, 42> debug_methods = {
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::delete_debug), // 0.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::debug_name), // 1.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::debug_is_type), // 2.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::debug_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_interface), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_interface), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_interface), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::handle_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::draw_debug), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::ime::initialize_debug), // 21.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::interface_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::interface_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
};

static void *create_debug(void *parent) {
    auto *screen =
        static_cast<DebugScreen *>(allocate_flags(sizeof(DebugScreen), 0, "GIMEDebugScreen"));
    if (!screen)
        return nullptr;
    try {
        return construct_debug(screen, parent);
    } catch (...) {
        release_flags(screen, 0, "GIMEDebugScreen");
        throw;
    }
}
static ClassRecord debug_class;
static CallbackRecord debug_callbacks[5];

static void unregister_debug_callbacks() {
    for (unsigned index = 5; index;)
        unregister_callback(&debug_callbacks[--index]);
}

static void register_debug_callbacks() {
    register_callback(&debug_callbacks[0], family::button_class_key, "BEVENT_PRESSED", 43,
                      "G_EnableIMEToggle", enable_callback, 0);
    register_callback(&debug_callbacks[1], family::button_class_key, "BEVENT_PRESSED", 43,
                      "G_ActivateIMEToggle", activate_callback, 0);
    register_callback(&debug_callbacks[2], family::button_class_key, "BEVENT_PRESSED", 43,
                      "G_DumpDebugInfo", information_callback, 0);
    register_callback(&debug_callbacks[3], family::button_class_key, "BEVENT_PRESSED", 43,
                      "G_EnglishHanjaIMEToggle", settings_callback, 0);
    register_callback(&debug_callbacks[4], family::button_class_key, "BEVENT_PRESSED", 43,
                      "G_PopulateCandidates", candidates_callback, 0);
    std::atexit(unregister_debug_callbacks);
}

static void unregister_debug() {
    unregister_class(&debug_class);
}

static void register_debug() {
    register_class(&debug_class, "GIMEDebugScreen", create_debug, "GInterface");
    std::atexit(unregister_debug);
}
} // namespace nfs::gui::widgets::ime
NFS_KERNEL_INITIALIZER(startup_register_debug_callbacks, ".CRT$XCU_NFS_00690098",
                       nfs::gui::widgets::ime::register_debug_callbacks);
NFS_KERNEL_INITIALIZER(startup_register_debug, ".CRT$XCU_NFS_0069009C",
                       nfs::gui::widgets::ime::register_debug);
