#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../../gui/widgets/gui_widget_shared_methods.hpp"
#include "frontend_menu_controls.hpp"
#include "../../gui/widgets/gui_widget_interaction.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::gui::widgets::family {

void set_slider_bar_position(SliderBar *slider, int position) {
    const int maximum = slider->maximum;
    const int minimum = slider->minimum > maximum ? maximum : slider->minimum;
    if (position < minimum)
        slider->position = minimum;
    else if (position > maximum)
        slider->position = maximum;
    else
        slider->position = position;
}
} // namespace nfs::gui::widgets::family
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
using game::runtime::system;
struct MenuSlider {
    family::SliderBar base;
    Dword binding;
    int default_value;
};
static_assert(sizeof(MenuSlider) == 332 && offsetof(MenuSlider, binding) == 324);
extern const std::array<Method, 48> menu_slider_methods;
static const gui::engine::pending::FlagDefinition slider_bindings[] = {
    {"kOptionsAudioMenuMusicVolume", 0, 48},
    {"kOptionsAudioRaceMusicVolume", 0, 49},
    {"kOptionsAudioEngineVolume", 0, 50},
    {"kOptionsAudioVoiceVolume", 0, 51},
    {"kOptionsAudioSfxVolume", 0, 52},
    {"kOptionsGraphicsReflection", 0, 31},
    {"kOptionsGraphicsScreenRes", 0, 29},
    {"kOptionsGraphicsTrackDetail", 0, 32},
    {"kOptionsGraphicsCarDetail", 0, 33},
    {"kOptionsGraphicsSpecialEffects", 0, 34},
    {"kOptionsGraphicsViewAngle", 0, 35},
    {"kOptionsGraphicsHorizon", 0, 36},
    {"kOptionsGraphicsGamma", 0, 37},
    {"kOptionsFFBStickVolume", 0, 53},
    {"kOptionsFFBRoadEffects", 0, 54},
    {"kOptionsFFBCollision", 0, 55},
    {"kOptionsFFBEngine", 0, 56},
    {"kOptionsFFBRoadGrip", 0, 57},
    {nullptr, 0, 0}};

static const char *__cdecl menu_slider_class_key() {
    return "CmnSlider";
}

static const char *__fastcall menu_slider_name(Widget *, void *) {
    return "CmnSlider";
}

static Dword __fastcall menu_slider_is_type(Widget *, void *, const void *type) {
    return type == reinterpret_cast<const void *>(menu_slider_class_key) ||
           type == reinterpret_cast<const void *>(family::slider_bar_class_key) ||
           type == reinterpret_cast<const void *>(general_class_name) ||
           type == reinterpret_cast<const void *>(base_class_name);
}

static Dword __fastcall menu_slider_is_name(Widget *, void *, const char *name) {
    return !std::strncmp(name, "CmnSlider", 10) || !std::strncmp(name, "GSliderBar", 11) ||
           !std::strncmp(name, "GWidget", 8) || !std::strncmp(name, "GUI", 4);
}

static MenuSlider *construct_menu_slider(MenuSlider *widget, void *parent) {
    gui::widgets::pending::construct_slider_bar(widget, parent);
    widget->binding = 0;
    widget->default_value = 0;
    widget->base.base.base.methods = menu_slider_methods.data();
    return widget;
}

static void destroy_menu_slider(MenuSlider *widget) {
    widget->base.base.base.methods = menu_slider_methods.data();
    family::slider_bar_destroy(&widget->base.base.base);
}

static MenuSlider *__fastcall delete_menu_slider(MenuSlider *widget, void *, Dword flags) {
    destroy_menu_slider(widget);
    if (flags & 1)
        gui::release(widget);
    return widget;
}

static void set_menu_slider_position(MenuSlider *widget, int position) {
    family::set_slider_bar_position(&widget->base, position);
    if (widget->binding)
        network::packets::pending::set_rule_variant(system(72), int(widget->binding),
                                                             Dword(widget->base.position));
}

static Dword __fastcall menu_slider_key(MenuSlider *widget, void *, Dword key, Dword modifiers) {
    Dword position;
    switch (key) {
    case 12:
    case 74:
    case 203:
        position = Dword(widget->base.position) - Dword(widget->base.step);
        break;
    case 13:
    case 78:
    case 205:
        position = Dword(widget->base.position) + Dword(widget->base.step);
        break;
    default:
        return gui::widgets::pending::general_key(&widget->base.base, nullptr, key, modifiers);
    }
    set_menu_slider_position(widget, int(position));
    gui::events::post_widget(widget, 90, 0, 0);
    return 1;
}

static void __fastcall prepare_menu_slider(MenuSlider *widget, void *,
                                           gui::engine::Engine *render) {
    family::slider_bar_prepare_draw(&widget->base.base.base, nullptr,
                                    reinterpret_cast<Dword>(render));
    if (auto *title = static_cast<Widget *>(pending::find_child(widget, "<TITLE>"))) {
        invoke(render, 21, family::inherited_palette(title));
        auto *active =
            static_cast<Widget *>(gui::events::find_widget(gui::context_state, active_widget_id));
        if (active && descendant(&widget->base.base.base, active->id)) {
            Dword colour;
            title->colour = *invoke<Dword *>(render, 23, &colour, Dword(1));
        }
    }
}

static Dword __fastcall load_menu_slider(MenuSlider *widget, void *, void *configuration) {
    gui::engine::pending::read_enum(configuration, "CmnSlider.mDataEnum", &widget->binding,
                                    slider_bindings, 0);
    gui::engine::pending::read_int(configuration, "CmnSlider.mDefault", &widget->default_value, 0);
    return family::slider_bar_load(&widget->base.base.base, nullptr,
                                   reinterpret_cast<Dword>(configuration));
}

static Dword __fastcall save_menu_slider(MenuSlider *widget, void *, void *configuration) {
    gui::engine::pending::write_enum(configuration, "CmnSlider.mDataEnum", widget->binding,
                                     slider_bindings, 0);
    gui::engine::pending::write_int(configuration, "CmnSlider.mDefault", widget->default_value, 0);
    return family::slider_bar_save(&widget->base.base.base, nullptr,
                                   reinterpret_cast<Dword>(configuration));
}

static void __fastcall initialize_menu_slider(MenuSlider *widget, void *) {
    family::slider_bar_initialize(&widget->base.base.base, nullptr);
    if (widget->binding)
        set_menu_slider_position(
            widget, int(network::packets::rule_value(system(72), int(widget->binding))));
}

static void __fastcall menu_slider_child_event(MenuSlider *widget, void *, Widget *sender,
                                               Dword event, Dword first, Dword second) {
    family::slider_bar_configure_geometry(&widget->base.base.base, nullptr,
                                          reinterpret_cast<Dword>(sender), event, first, second);
}

static Dword __fastcall menu_slider_event(MenuSlider *widget, void *, Dword event, Dword first,
                                         Dword second) {
    if (event == 90 || (event == 103 && widget->binding && first == widget->binding)) {
        if (event == 103)
            family::set_slider_bar_position(&widget->base, int(second));
        if (widget->binding)
            network::packets::pending::set_rule_variant(system(72), int(widget->binding),
                                                                 Dword(widget->base.position));
    }
    return family::slider_bar_event(&widget->base.base.base, nullptr, event, first, second);
}

const std::array<Method, 48> menu_slider_methods = {
    reinterpret_cast<Method>(::nfs::application::frontend::delete_menu_slider), // 0.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_slider_name), // 1.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_slider_is_type), // 2.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_slider_is_name), // 3.
    reinterpret_cast<Method>(::nfs::application::frontend::load_menu_slider), // 4.
    reinterpret_cast<Method>(::nfs::application::frontend::save_menu_slider), // 5.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::layout), // 6.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_text), // 7.
    reinterpret_cast<Method>(::nfs::gui::widgets::text), // 8.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::hit_test), // 9.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::navigate), // 10.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::enter_modal), // 11.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::leave_modal), // 12.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_slider_event), // 13.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_slider_child_event), // 14.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_two_arguments), // 15.
    reinterpret_cast<Method>(::nfs::gui::widgets::apply_drawing_state), // 16.
    reinterpret_cast<Method>(::nfs::application::frontend::prepare_menu_slider), // 17.
    reinterpret_cast<Method>(::nfs::gui::widgets::push_draw_bounds), // 18.
    reinterpret_cast<Method>(::nfs::gui::widgets::draw_content), // 19.
    reinterpret_cast<Method>(::nfs::gui::widgets::pop_draw_bounds), // 20.
    reinterpret_cast<Method>(::nfs::application::frontend::initialize_menu_slider), // 21.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_no_arguments), // 22.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_render_argument), // 23.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_style_argument), // 24.
    reinterpret_cast<Method>(::nfs::gui::widgets::clear_layout_request), // 25.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_layout_argument), // 26.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::slider_bar_parent_enter), // 27.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_parent_leave), // 28.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_parent_argument), // 29.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_child_argument), // 30.
    reinterpret_cast<Method>(::nfs::gui::widgets::set_modal_state), // 31.
    reinterpret_cast<Method>(::nfs::application::frontend::menu_slider_key), // 32.
    reinterpret_cast<Method>(::nfs::gui::widgets::reserved_result), // 33.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_34), // 34.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_35), // 35.
    reinterpret_cast<Method>(::nfs::gui::widgets::pending::general_button), // 36.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_37), // 37.
    reinterpret_cast<Method>(::nfs::gui::widgets::style_result_38), // 38.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::slider_bar_pointer_move), // 39.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::slider_bar_pointer_press), // 40.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::slider_bar_pointer_release), // 41.
    reinterpret_cast<Method>(::nfs::gui::widgets::accept_general), // 42.
    reinterpret_cast<Method>(::nfs::gui::widgets::cancel_general), // 43.
    reinterpret_cast<Method>(::nfs::gui::widgets::notify_scroll), // 44.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_command), // 45.
    reinterpret_cast<Method>(::nfs::gui::widgets::process_general_notification), // 46.
    reinterpret_cast<Method>(::nfs::gui::widgets::family::slider_bar_item_operation_47), // 47.
};

static void *create_menu_slider(void *parent) {
    auto *storage =
        static_cast<MenuSlider *>(gui::allocate_flags(sizeof(MenuSlider), 0, "CmnSlider"));
    if (!storage)
        return nullptr;
    try {
        return construct_menu_slider(storage, parent);
    } catch (...) {
        gui::release(storage);
        throw;
    }
}
static ClassRecord menu_slider_class;

static void unregister_menu_slider() {
    unregister_class(&menu_slider_class);
}

static void register_menu_slider() {
    register_class(&menu_slider_class, "CmnSlider", create_menu_slider, "GSliderBar");
    std::atexit(unregister_menu_slider);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_menu_slider, ".CRT$XCU_NFS_00690060",
                       register_menu_slider);
} // namespace
} // namespace nfs::application::frontend
