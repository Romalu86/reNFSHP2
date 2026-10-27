#pragma once
#include "gui_widget_interaction.hpp"
#include "gui_widget_text.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui::widgets::family {
struct EditText {
    GeneralWidget base;
    Dword text_style, password_character;
    int margin, line_spacing, caret, anchor;
    String saved_text;
    int character_limit;
    const char *alphabet;
    int alphabet_position;
    Dword filter, edit_style;
};
static_assert(sizeof(EditText) == 328 && offsetof(EditText, caret) == 280 &&
              offsetof(EditText, edit_style) == 324);
inline EditText *edit(Widget *widget) {
    return reinterpret_cast<EditText *>(widget);
}
struct EditString : String {
    EditString() {
        string_construct(this);
    }
    ~EditString() {
        string_destruct(this);
    }
    EditString(const EditString &) = delete;
    EditString &operator=(const EditString &) = delete;
};
void set_edit_anchor(Widget *, int);
void set_edit_caret(Widget *, int, Dword);
int edit_caret(Widget *);
int edit_anchor(Widget *);
Point *edit_position_to_screen(Widget *, Point *, int);
int edit_screen_to_position(Widget *, const Point *);
Dword edit_text_fits(Widget *, const String *);
void erase_edit_selection(Widget *);
Dword insert_edit_character(Widget *, Dword, Dword);
Dword overwrite_edit_character(Widget *, Dword, Dword);
Dword take_widget_event(Widget *, Dword *, Dword *, Dword *, int, int, Dword);
void backspace_edit_character(Widget *);
Dword paste_edit_text(Widget *);
String *selected_edit_text(Widget *, String *);
Dword select_edit_word(Widget *, int);
inline const String *edit_value(Widget *widget) {
    return game::runtime::invoke<const String *>(widget, 8);
}
// Common spelling of the repeated caret/selection stores in the saved methods.
inline void move_edit_selection(Widget *widget, int position, bool move_anchor) {
    if (active_widget_id == widget->id && position != edit(widget)->caret)
        if (auto *render = active_engine())
            game::runtime::invoke(render, 60);
    edit(widget)->caret = clamp_gui_integer(position, 0, edit_value(widget)->length);
    if (!(edit(widget)->edit_style & 8) || move_anchor)
        set_edit_anchor(widget, edit(widget)->caret);
}
inline bool edit_layout_flag(Widget *widget, Dword flag) {
    return (widget->layout_flags & flag) != 0;
}
} // namespace nfs::gui::widgets::family
namespace nfs::gui {
extern String *clipboard_text;
extern Dword clipboard_enabled;
void set_clipboard_enabled(Dword);
Dword copy_clipboard(const String *);
Dword read_clipboard(String *);
Dword clear_clipboard();
void erase_string_character(String *, int);
void insert_string_character(String *, int, char);
void insert_string_wide_character(String *, int, wchar_t);
} // namespace nfs::gui
namespace nfs::gui::engine::pending {
Dword character_at(const String *, int);
}
