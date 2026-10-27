#pragma once
#include "../flow/frontend_runtime.hpp"
#include "../../gui/widgets/gui_widget_controls.hpp"
#include "../../gui/widgets/gui_widget_hooks.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::application::pending {
using Widget = gui::widgets::Widget;
struct MessageBox {
    const gui::Destroy *methods;
    Widget *widget;
    Dword buttons[4];
    int button_count;
    Dword flags;
    int base_height;
};
static_assert(sizeof(MessageBox) == 36 && offsetof(MessageBox, flags) == 28);
extern const gui::Destroy message_box_methods[1];
Widget *widget_root(Widget *);
void set_widget_tab_order(Widget *, Dword);
Dword set_widget_callback(Widget *, Dword, gui::widgets::Callback);
Widget *create_message_widget(const gui::String *, const gui::String *, const gui::String *, void *,
                              const gui::String *, const gui::String *);
void initialize_message_box(MessageBox *, const gui::String *, const gui::String *, Dword);
MessageBox *construct_message_box(MessageBox *, const gui::String *, const gui::String *);
MessageBox *construct_message_box(MessageBox *, const gui::String *);
MessageBox *construct_message_box(MessageBox *, const gui::String *, Dword);
MessageBox *construct_message_box(MessageBox *, const gui::String *, const gui::String *,
                           Dword);
void destroy_message_box(MessageBox *);
int run_message_box(MessageBox *, gui::ModalCallback, Dword);
Dword configure_message_button(MessageBox *, int, const gui::String *, Dword);
Dword __cdecl accept_message_button(void *, Dword, Dword, Dword);
gui::engine::Rectangle *copy_dialog_rectangle(gui::engine::Rectangle *,
                                              const gui::engine::Rectangle *);
gui::engine::Point *construct_dialog_point(gui::engine::Point *, int, int);
int prompt_text_entry(Widget *, gui::String, gui::String, gui::String *, const gui::String *, int,
                      gui::String, Dword, gui::ModalCallback, Dword, Dword);
Widget *create_text_entry_dialog(Widget *, gui::String, gui::String, gui::String *, int,
                                 gui::String, Dword, Dword);
void configure_text_entry_dialog(Widget *, gui::String, gui::String);
void configure_dialog_text(Widget *, gui::String, gui::String);
// Stack owners use the same destructor on every ordinary/catch exit.
struct ScopedMessageBox {
    MessageBox value;
    ScopedMessageBox(const gui::String *text, Dword flags) {
        construct_message_box(&value, text, flags);
    }
    ~ScopedMessageBox() {
        destroy_message_box(&value);
    }
};
} // namespace nfs::application::pending
