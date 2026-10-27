#include "frontend_runtime.hpp"
#include "../../gui/widgets/gui_widget_edit.hpp"
namespace nfs::application::pending {
gui::String *construct_integer_text(gui::String *, int);

gui::String *construct_grouped_integer_text(gui::String *destination, int value) {
    gui::String temporary;
    construct_integer_text(&temporary, value);
    try {
        if (value > 999999)
            gui::insert_string_character(&temporary, temporary.length - 6, ',');
        if (value > 999)
            gui::insert_string_character(&temporary, temporary.length - 3, ',');
        gui::string_construct_copy(destination, &temporary);
    } catch (...) {
        gui::string_destruct(&temporary);
        throw;
    }
    gui::string_destruct(&temporary);
    return destination;
}
} // namespace nfs::application::pending
