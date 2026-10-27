#include "gui_widget_controls.hpp"
#include "gui_widget_family_style.hpp"
#include "nfs/gui/resources/gui_format.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
using game::runtime::at;
using game::runtime::bytes;
using game::runtime::invoke;
static String *tab_name(Widget *widget) {
    return reinterpret_cast<String *>(bytes(widget) + 312);
}
static String *tab_resource(Widget *widget) {
    return reinterpret_cast<String *>(bytes(widget) + 336);
}
// Compare narrow characters as signed values.
static bool equal_tab_name(const String *first, const String *second) {
    const void *a = string_data(first);
    if (!a)
        return !string_data(second);
    const void *b = string_data(second);
    if (!b)
        return !string_data(first);
    if (first->length != second->length)
        return false;
    a = string_data(first);
    b = string_data(second);
    for (Dword index = 0;; ++index) {
        const int left = first->wide ? static_cast<const std::uint16_t *>(a)[index]
                                     : static_cast<const signed char *>(a)[index];
        const int right = second->wide ? static_cast<const std::uint16_t *>(b)[index]
                                       : static_cast<const signed char *>(b)[index];
        if (left != right)
            return false;
        if (!left)
            return true;
    }
}

Dword __fastcall tab_button_load(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    const auto *empty = &lifecycle::construction::unresolved::shared_string;
    engine::pending::read_string(configuration, "GTabButton.mTabView", tab_name(widget), empty);
    engine::pending::read_flags(configuration, "GTabButtom.mTabButtonStyle",
                                &at<Dword>(widget, 332), tbs_bringtotop_flags, 0);
    engine::pending::read_string(configuration, "GTabButton.mDynamicLoad", tab_resource(widget),
                                 empty);
    if (equal_tab_name(tab_name(widget), &widget->name))
        string_copy(tab_name(widget), empty);
    return button_load(widget, nullptr, address);
}

Dword __fastcall tab_button_save(Widget *widget, void *, Dword address) {
    void *configuration = reinterpret_cast<void *>(address);
    engine::pending::write_text(configuration, "GTabButton.mTabView",
                                string_ascii(tab_name(widget)), nullptr);
    engine::pending::write_flags(configuration, "GTabButtom.mTabButtonStyle",
                                 at<Dword>(widget, 332), tbs_bringtotop_flags, 0);
    engine::pending::write_ascii(configuration, "GTabButton.mDynamicLoad",
                                 string_ascii(tab_resource(widget)), "FILE", 0, nullptr);
    engine::pending::write_flags(configuration, "GButton.mButtonStyle", at<Dword>(widget, 296),
                                 bs_greybutton_flags, 0);
    engine::pending::write_int(configuration, "GButton.mRadioGroup", at<int>(widget, 304), 0);
    return pending::save_general(reinterpret_cast<GeneralWidget *>(widget), nullptr, configuration);
}
static void apply_tab_visibility(Widget *widget, Dword visible) {
    Widget *view = named_sibling(widget, tab_name(widget));
    if (!view || view == widget)
        return;
    if (tab_resource(widget)->length && visible && !view->children.count) {
        destroy_widget_tree(view);
        set_widget_resource(view, tab_resource(widget));
        application::pending::load_widget_children(view, 1);
        initialize_tree(view);
        set_widget_resource(view, &lifecycle::construction::unresolved::shared_string);
    }
    set_enabled(view, visible);
    application::pending::set_widget_visible(view, visible);
    events::post_widget(view, 57, visible, 0);
}

void __fastcall tab_button_selection_changed(Widget *widget, void *, Dword selected) {
    apply_tab_visibility(widget, selected);
    if (selected && (at<Dword>(widget, 332) & 1))
        move_to_first_draw_position(widget);
}

void __fastcall set_tab_view(Widget *widget, void *, Dword name_address) {
    const auto *name = reinterpret_cast<const String *>(name_address);
    if (Widget *view = named_sibling(widget, name); view && view != widget)
        string_copy(tab_name(widget), name);
    apply_tab_visibility(widget, reinterpret_cast<Button *>(widget)->checked);
}

void __fastcall tab_button_initialize(Widget *widget, void *) {
    apply_tab_visibility(widget, reinterpret_cast<Button *>(widget)->checked);
    if (reinterpret_cast<Button *>(widget)->checked && (at<Dword>(widget, 332) & 1))
        move_to_first_draw_position(widget);
    button_initialize(widget, nullptr);
}
} // namespace nfs::gui::widgets::family
