#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/core/lifecycle/array_construction.hpp"
#include <cstdlib>
#include "gui_widgets.hpp"
#include "nfs/core/application/application.hpp"
#include "../lifecycle/gui_configuration_cache.hpp"
namespace nfs::gui::widgets {
ClassRecord *classes{};
namespace pending {
void *construct_simple_text(void *, void *);
void *construct_text(void *, void *);
void *construct_edit_text(void *, void *);
void *construct_tool_tip(void *, void *);
void *construct_static_image(void *, void *);
void *construct_anim_image(void *, void *);
void *construct_image_box(void *, void *);
void *construct_image_bar(void *, void *);
void *construct_image_list(void *, void *);
void *construct_button(void *, void *);
void *construct_toggle_button(void *, void *);
void *construct_tab_button(void *, void *);
void *construct_spin_button(void *, void *);
void *construct_list_box(void *, void *);
void *construct_combo_box(void *, void *);
void *construct_tree(void *, void *);
void *construct_frame(void *, void *);
void *construct_scroll_bar(void *, void *);
void *construct_scroll_view(void *, void *);
void *construct_text_view(void *, void *);
void *construct_gripper(void *, void *);
void *construct_slider_bar(void *, void *);
void *construct_menu(void *, void *);
void *construct_dialog(void *, void *);
void *construct_msg_box(void *, void *);
void *construct_marquee(void *, void *);
void *construct_interface(void *, void *);
} // namespace pending

ClassRecord *register_class(ClassRecord *record, const char *name, Create create, const char *base_name) {
    record->create = create;
    record->next = nullptr;
    record->name = name;
    record->base_name = base_name;
    engine::pending::hash_name(&record->hash, name);
    record->next = classes;
    classes = record;
    return record;
}

ClassRecord *copy_class(ClassRecord *record, const ClassRecord *source) {
    record->name = source->name;
    record->create = source->create;
    record->hash = source->hash;
    record->base_name = source->base_name;
    record->next = classes;
    classes = record;
    return record;
}

void link_class(ClassRecord *record) {
    record->next = classes;
    classes = record;
}

void unregister_class(ClassRecord *record) {
    ClassRecord *previous = nullptr;
    for (ClassRecord *current = classes; current; current = current->next) {
        if (current == record) {
            if (previous)
                previous->next = record->next;
            else
                classes = record->next;
            return;
        }
        previous = current;
    }
}

static void *create_widget(void *parent) {
    void *storage = allocate_flags(264, 0, "GWidget");
    if (!storage)
        return nullptr;
    try {
        return construct_widget(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_simple_text(void *parent) {
    void *storage = allocate_flags(256, 0, "GSimpleText");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_simple_text(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_simple_image(void *parent) {
    void *storage = allocate_flags(296, 0, "GSimpleImage");
    if (!storage)
        return nullptr;
    try {
        return construct_simple_image(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

void *create_text(void *parent) {
    void *storage = allocate_flags(280, 0, "GText");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_text(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_edit_text(void *parent) {
    void *storage = allocate_flags(328, 0, "GEditText");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_edit_text(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

void *create_tool_tip(void *parent) {
    void *storage = allocate_flags(288, 0, "GToolTip");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_tool_tip(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_static_image(void *parent) {
    void *storage = allocate_flags(324, 0, "GStaticImage");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_static_image(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_anim_image(void *parent) {
    void *storage = allocate_flags(400, 0, "GAnimImage");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_anim_image(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

void *create_image_box(void *parent) {
    void *storage = allocate_flags(288, 0, "GImageBox");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_image_box(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_image_bar(void *parent) {
    void *storage = allocate_flags(284, 0, "GImageBar");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_image_bar(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_image_list(void *parent) {
    void *storage = allocate_flags(388, 0, "GImageList");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_image_list(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

void *create_button(void *parent) {
    void *storage = allocate_flags(312, 0, "GButton");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_button(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_toggle_button(void *parent) {
    void *storage = allocate_flags(356, 0, "GToggleButton");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_toggle_button(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_tab_button(void *parent) {
    void *storage = allocate_flags(356, 0, "GTabButton");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_tab_button(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_spin_button(void *parent) {
    void *storage = allocate_flags(300, 0, "GSpinButton");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_spin_button(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_list_box(void *parent) {
    void *storage = allocate_flags(368, 0, "GListBox");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_list_box(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_combo_box(void *parent) {
    void *storage = allocate_flags(344, 0, "GComboBox");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_combo_box(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_tree(void *parent) {
    void *storage = allocate_flags(356, 0, "GTree");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_tree(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_frame(void *parent) {
    void *storage = allocate_flags(320, 0, "GFrame");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_frame(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

void *create_scroll_bar(void *parent) {
    void *storage = allocate_flags(336, 0, "GScrollBar");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_scroll_bar(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_scroll_view(void *parent) {
    void *storage = allocate_flags(300, 0, "GScrollView");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_scroll_view(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_text_view(void *parent) {
    void *storage = allocate_flags(304, 0, "GTextView");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_text_view(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_gripper(void *parent) {
    void *storage = allocate_flags(404, 0, "GGripper");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_gripper(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_slider_bar(void *parent) {
    void *storage = allocate_flags(324, 0, "GSliderBar");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_slider_bar(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_menu(void *parent) {
    void *storage = allocate_flags(364, 0, "GMenu");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_menu(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_dialog(void *parent) {
    void *storage = allocate_flags(288, 0, "GDialog");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_dialog(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_msg_box(void *parent) {
    void *storage = allocate_flags(296, 0, "GMsgBox");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_msg_box(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_marquee(void *parent) {
    void *storage = allocate_flags(304, 0, "GMarquee");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_marquee(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}

static void *create_interface(void *parent) {
    void *storage = allocate_flags(256, 0, "GInterface");
    if (!storage)
        return nullptr;
    try {
        return pending::construct_interface(storage, parent);
    } catch (...) {
        release(storage);
        throw;
    }
}
static ClassRecord builtin_classes[29];
static void unregister_builtin_classes();
// Registration order is the order in the initializer.
static void register_builtin_classes() {
    register_class(builtin_classes + 0, "GWidget", create_widget, nullptr);
    register_class(builtin_classes + 1, "GSimpleText", create_simple_text, nullptr);
    register_class(builtin_classes + 2, "GSimpleImage", create_simple_image, nullptr);
    register_class(builtin_classes + 3, "GText", create_text, "GWidget");
    register_class(builtin_classes + 4, "GEditText", create_edit_text, "GText");
    register_class(builtin_classes + 5, "GToolTip", create_tool_tip, "GWidget");
    register_class(builtin_classes + 6, "GStaticImage", create_static_image, "GWidget");
    register_class(builtin_classes + 7, "GAnimImage", create_anim_image, "GStaticImage");
    register_class(builtin_classes + 8, "GImageBox", create_image_box, "GWidget");
    register_class(builtin_classes + 9, "GImageBar", create_image_bar, "GWidget");
    register_class(builtin_classes + 10, "GImageList", create_image_list, "GWidget");
    register_class(builtin_classes + 11, "GButton", create_button, "GWidget");
    register_class(builtin_classes + 12, "GToggleButton", create_toggle_button, "GButton");
    register_class(builtin_classes + 13, "GTabButton", create_tab_button, "GButton");
    register_class(builtin_classes + 14, "GSpinButton", create_spin_button, "GWidget");
    register_class(builtin_classes + 15, "GListBox", create_list_box, "GWidget");
    register_class(builtin_classes + 16, "GComboBox", create_combo_box, "GWidget");
    register_class(builtin_classes + 17, "GTree", create_tree, "GWidget");
    register_class(builtin_classes + 18, "GFrame", create_frame, "GWidget");
    register_class(builtin_classes + 19, "GScrollBar", create_scroll_bar, "GWidget");
    register_class(builtin_classes + 20, "GScrollView", create_scroll_view, "GWidget");
    register_class(builtin_classes + 21, "GTextView", create_text_view, "GScrollView");
    register_class(builtin_classes + 22, "GGripper", create_gripper, "GAnimImage");
    register_class(builtin_classes + 23, "GSliderBar", create_slider_bar, "GWidget");
    register_class(builtin_classes + 24, "GMenu", create_menu, "GWidget");
    register_class(builtin_classes + 25, "GDialog", create_dialog, "GWidget");
    register_class(builtin_classes + 26, "GMsgBox", create_msg_box, "GDialog");
    register_class(builtin_classes + 27, "GMarquee", create_marquee, "GWidget");
    register_class(builtin_classes + 28, "GInterface", create_interface, nullptr);
    std::atexit(unregister_builtin_classes);
}
// The native iterator walks all29 records backwards.
static void unregister_builtin_classes() {
    lifecycle::construction::objects::destroy_array(builtin_classes, sizeof(ClassRecord), 29,
                                                       unregister_class);
}
NFS_KERNEL_INITIALIZER(startup_register_builtin_classes, ".CRT$XCU_NFS_006906E4",
                       register_builtin_classes);
} // namespace nfs::gui::widgets
namespace nfs::application::pending {

void *create_widget_class(const char *name, void *parent) {
    if (!name)
        return nullptr;
    Dword hash;
    gui::engine::pending::hash_name(&hash, name);
    if (!hash)
        return nullptr;
    for (gui::widgets::ClassRecord *record = gui::widgets::classes; record; record = record->next)
        if (record->hash == hash)
            return record->create(parent);
    return nullptr;
}
} // namespace nfs::application::pending
