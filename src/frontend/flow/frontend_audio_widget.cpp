#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "frontend_application_widgets.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "../../gui/lifecycle/gui_configuration_values.hpp"
#include "nfs/core/errors/assertions.hpp"
#include "nfs/audio/music/music_clients.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
struct AudioEffect {
    Widget base;
    Dword audio_type;
    std::uint8_t initialized, padding[3];
};
static_assert(sizeof(AudioEffect) == 264);
static const gui::engine::pending::FlagDefinition audio_types[] = {
    {"MAUDIO_BUTTON", 0, 0},
    {"MAUDIO_LISTBOX", 0, 1},
    {"MAUDIO_COMBO", 0, 2},
    {"MAUDIO_MENU", 0, 3},
    {"MAUDIO_CAMERABUTTON", 0, 4},
    {"MAUDIO_PAYCHECK", 0, 5},
    {"MAUDIO_CHECKBOX", 0, 6},
    {"MAUDIO_BUTTONFOCUS", 0, 7},
    {"MAUDIO_TOGGLE", 0, 8},
    {"MAUDIO_SPIN", 0, 9},
    {"MAUDIO_WHEEL", 0, 10},
    {"MAUDIO_SCROLLBUTTON", 0, 11},
    {nullptr, 0, 0}};
extern const std::array<Method, 42> audio_base_methods;
extern const std::array<Method, 43> audio_effect_base_methods, menu_audio_methods;

static const char *__cdecl audio_class_key() {
    return "GAudio";
}

static const char *__cdecl audio_effect_class_key() {
    return "GAudioEffect";
}

static const char *__cdecl menu_audio_class_key() {
    return "MAudioFX";
}

static const char *__fastcall menu_audio_name(Widget *, void *) {
    return "MAudioFX";
}

static Dword __fastcall audio_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GAudio", 7) || !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall audio_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(audio_class_key) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Widget *__fastcall delete_audio_base(Widget *widget, void *, Dword flags) {
    widget->methods = audio_base_methods.data();
    destroy_base(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

const std::array<Method, 42> audio_base_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_audio_base), // 0.
    reinterpret_cast<Method>(::nfs::assertions::pure_virtual_call), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_configuration), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_configuration), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::handle_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::gui::widgets::request_layout), // 21.
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
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_32), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_36), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
};

static void destroy_audio_effect_base(Widget *widget) {
    widget->methods = audio_base_methods.data();
    destroy_base(widget);
}

static Widget *__fastcall delete_audio_effect_base(Widget *widget, void *, Dword flags) {
    destroy_audio_effect_base(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static Dword __fastcall audio_effect_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "GAudioEffect", 13) || !std::strncmp(name, "GAudio", 7) ||
           !std::strncmp(name, "GUI", 4);
}

static Dword __fastcall audio_effect_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(audio_effect_class_key) ||
           type == reinterpret_cast<const void *>(audio_class_key) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static void __fastcall initialize_audio_effect_base(Widget *widget, void *) {
    if (widget->parent)
        widget->parent->value_d8 = widget->id;
    request_layout(widget, nullptr);
}

static void __fastcall release_audio_effect_parent(Widget *widget, void *) {
    if (widget->parent && widget->parent->value_d8 == widget->id)
        widget->parent->value_d8 = 0;
    clear_layout_request(widget, nullptr);
}

static Dword __fastcall audio_effect_event(Widget *widget, void *, Dword event, Dword first,
                                          Dword second) {
    if (event != 36)
        return gui::widgets::pending::handle_event(widget, nullptr, event, first, second);
    if (first) {
        const auto *notification = reinterpret_cast<const Dword *>(first);
        invoke(widget, 42, notification[0], notification[1], notification[2], notification[3]);
    }
    return 1;
}

const std::array<Method, 43> audio_effect_base_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_audio_effect_base), // 0.
    reinterpret_cast<Method>(::nfs::assertions::pure_virtual_call), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_effect_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_effect_is_name), // 3.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::load_configuration), // 4.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::save_configuration), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_effect_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_audio_effect_base), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_audio_effect_parent), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_32), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_36), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::assertions::pure_virtual_call), // 42.
};

static Widget *construct_audio_effect_base(void *storage, void *parent) {
    auto *widget = static_cast<Widget *>(construct_base(storage, parent));
    widget->flags = (widget->flags & ~Dword(4)) | 3;
    widget->methods = audio_effect_base_methods.data();
    return widget;
}

static AudioEffect *construct_menu_audio(AudioEffect *widget, void *parent) {
    construct_audio_effect_base(widget, parent);
    widget->base.methods = menu_audio_methods.data();
    widget->audio_type = 0;
    widget->initialized = 0;
    return widget;
}

static void destroy_menu_audio(AudioEffect *widget) {
    widget->base.methods = menu_audio_methods.data();
    destroy_audio_effect_base(&widget->base);
}

static AudioEffect *__fastcall delete_menu_audio(AudioEffect *widget, void *, Dword flags) {
    destroy_menu_audio(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static Dword __fastcall menu_audio_is_type(Widget *widget, void *, const void *type) {
    return type == reinterpret_cast<const void *>(menu_audio_class_key) ||
           audio_effect_is_type(widget, nullptr, type);
}

static Dword __fastcall menu_audio_is_name(Widget *widget, void *, const char *name) {
    return !std::strncmp(name, "MAudioFX", 9) || audio_effect_is_name(widget, nullptr, name);
}

static void __fastcall initialize_menu_audio(AudioEffect *widget, void *) {
    initialize_audio_effect_base(&widget->base, nullptr);
    widget->base.flags = (widget->base.flags & ~Dword(4)) | 3;
}
// The dispatch table has twelve entries.
static void __fastcall menu_audio_notification(AudioEffect *widget, void *, Dword, Dword event,
                                               Dword first, Dword) {
    int sound = -1;
    switch (widget->audio_type) {
    case 0:
        if (event == 43)
            sound = int(network::rules::random() & 3);
        break;
    case 1:
        if (event == 40)
            sound = 0;
        break;
    case 2:
    case 7:
        if (event == 71)
            sound = 0;
        break;
    case 3:
        if (event == 17 && widget->initialized)
            sound = 16;
        else if (event == 19) {
            widget->initialized = 1;
            sound = 32;
        }
        break;
    case 4:
        if (event == 43)
            sound = 0;
        break;
    case 5:
        if (event == 79)
            sound = 0;
        break;
    case 6:
        if (event == 43)
            sound = 16;
        break;
    case 8:
        if (event == 43 || (event == 6 && (first == 203 || first == 205)))
            sound = 32;
        break;
    case 9:
    case 10:
    case 11:
        if (event == 43)
            sound = 32;
        break;
    }
    if (sound >= 0)
        game::audio::play_frontend_event(sound, 127);
}

static Dword __fastcall load_menu_audio(AudioEffect *widget, void *, void *configuration) {
    gui::engine::pending::read_enum(configuration, "MAudioFX.mAudioType", &widget->audio_type,
                                    audio_types, 0);
    return gui::widgets::pending::load_configuration(&widget->base, nullptr, configuration);
}

static Dword __fastcall save_menu_audio(AudioEffect *widget, void *, void *configuration) {
    gui::engine::pending::write_enum(configuration, "MAudioFX.mAudioType", widget->audio_type,
                                     audio_types, 0);
    return gui::widgets::pending::save_configuration(&widget->base, nullptr, configuration);
}

const std::array<Method, 43> menu_audio_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_menu_audio), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_audio_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_audio_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_audio_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_menu_audio), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_menu_audio), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::audio_effect_event), // 13.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_four_arguments), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::gui::widgets::prepare_draw), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_menu_audio), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::application::frontend::release_audio_effect_parent), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_32), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_36), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_active_style), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::update_focus_style), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::capture_style), // 41.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_audio_notification), // 42.
};

static void *create_menu_audio(void *parent) {
    auto *storage =
        static_cast<AudioEffect *>(gui::allocate_flags(sizeof(AudioEffect), 0, "MAudioFX"));
    if (!storage)
        return nullptr;
    try {
        return construct_menu_audio(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord menu_audio_class;

static void unregister_menu_audio() {
    unregister_class(&menu_audio_class);
}

static void register_menu_audio() {
    register_class(&menu_audio_class, "MAudioFX", create_menu_audio, nullptr);
    std::atexit(unregister_menu_audio);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_menu_audio, ".CRT$XCU_NFS_006900DC",
                       register_menu_audio);
} // namespace
} // namespace nfs::application::frontend
