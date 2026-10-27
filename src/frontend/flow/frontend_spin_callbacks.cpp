#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "../screens/frontend_menu_controls.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "../../gui/widgets/gui_widget_hooks.hpp"
#include <cstdlib>
namespace nfs::application::frontend {
using namespace gui::widgets;
using game::runtime::invoke;
void select_type_spin(TypeSpinControl *, int);
static int previous_spin_row(TypeSpinControl *spin) {
    int row = family::previous_list_row(&spin->items, family::selected_list_row(&spin->items));
    if (row < 0) {
        row = int(spin->items.grid.rows.count) - 1;
        while (row >= 0 && family::list_row_disabled(&spin->items, Dword(row)))
            --row;
    }
    return row;
}
static int next_spin_row(TypeSpinControl *spin) {
    const int row = family::next_list_row(&spin->items, family::selected_list_row(&spin->items));
    return row < 0 ? 0 : row;
}
static void focus_spin_sibling(Widget *button, const char *name) {
    Widget *sibling;
    {
        pending::LocalFrontendString text(name);
        sibling = family::named_sibling(button, &text.value);
    }
    if (sibling)
        activate(sibling);
}

static Dword __cdecl spin_left_key(void *sender, Dword event, Dword key, Dword modifiers) {
    if (event != 6 || std::uint8_t(modifiers))
        return 0;
    auto *button = static_cast<Widget *>(sender);
    auto *parent = button->parent;
    if (!parent || !invoke<Dword>(parent, 2, type_spin_class_key))
        return 0;
    auto *spin = reinterpret_cast<TypeSpinControl *>(parent);
    if (key == 28 || key == 203)
        select_type_spin(spin, previous_spin_row(spin));
    else if (key == 205) {
        select_type_spin(spin, next_spin_row(spin));
        focus_spin_sibling(button, "<bR>");
    } else
        return 0;
    return 1;
}

static Dword __cdecl spin_right_key(void *sender, Dword event, Dword key, Dword modifiers) {
    if (event != 6 || std::uint8_t(modifiers))
        return 0;
    auto *button = static_cast<Widget *>(sender);
    auto *parent = button->parent;
    if (!parent || !invoke<Dword>(parent, 2, type_spin_class_key))
        return 0;
    auto *spin = reinterpret_cast<TypeSpinControl *>(parent);
    if (key == 28 || key == 205)
        select_type_spin(spin, next_spin_row(spin));
    else if (key == 203) {
        select_type_spin(spin, previous_spin_row(spin));
        focus_spin_sibling(button, "<bL>");
    } else
        return 0;
    return 1;
}
static CallbackRecord spin_callbacks[2];

static void unregister_spin_callbacks() {
    for (unsigned index = 2; index; --index)
        unregister_callback(spin_callbacks + index - 1);
}

static void register_spin_callbacks() {
    register_callback(spin_callbacks, family::button_class_key, "GEVENT_KEYDOWN", 6,
                      "TypeSpinCtl_Left", spin_left_key, 0);
    register_callback(spin_callbacks + 1, family::button_class_key, "GEVENT_KEYDOWN", 6,
                      "TypeSpinCtl_Right", spin_right_key, 0);
    std::atexit(unregister_spin_callbacks);
}
namespace {
NFS_KERNEL_INITIALIZER(startup_register_spin_callbacks, ".CRT$XCU_NFS_00690168",
                       register_spin_callbacks);
} // namespace
} // namespace nfs::application::frontend
