#pragma once
#include "gui_widget_controls.hpp"
#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::gui {
extern void *text_scratch;
extern Dword text_scratch_busy;
int string_find_character(const String *, Dword);
void string_append_character(String *, char);
} // namespace nfs::gui
namespace nfs::gui::engine {
const Rectangle *current_viewport_bounds(const Base *);
void intersect_in_place(Rectangle *, const Rectangle *);
Rectangle *translate_rectangle(Rectangle *, const Rectangle *, const Point *);
} // namespace nfs::gui::engine
namespace nfs::gui::widgets {
void union_rectangle(Rectangle *, const Rectangle *);
}
namespace nfs::gui::widgets::family {
Rectangle *text_content_bounds(Widget *, Rectangle *);
Dword first_text_line(Widget *, String *, Rectangle *, int *);
Dword next_text_line(Widget *, String *, Rectangle *, int *);
} // namespace nfs::gui::widgets::family
