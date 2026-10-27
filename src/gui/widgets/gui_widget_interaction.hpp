#pragma once
#include "gui_widget_text.hpp"
namespace nfs::gui::engine {
Rectangle *copy_rectangle_fields(Rectangle *, const Rectangle *);
Rectangle *union_copy(Rectangle *, const Rectangle *, const Rectangle *);
} // namespace nfs::gui::engine
namespace nfs::gui::widgets {
void *create_button(void *);
}
namespace nfs::gui::widgets::family {
int clamp_gui_integer(int, int, int);
long double clamp_gui_float(float, float, float);
int scale_gui_distance(int);
Point *to_screen_point(Widget *, Point *, const Point *);
Point *from_screen_point(Widget *, Point *, const Point *);
Rectangle *to_screen_rectangle(Widget *, Rectangle *, const Rectangle *);
Rectangle *absolute_widget_bounds(Widget *, Rectangle *);
void set_scroll_bar_position(Widget *, int);
struct ScrollBar {
    GeneralWidget base;
    Dword style;
    int maximum, minimum;
    float thumb_fraction;
    int position, thumb_offset;
    Rectangle track;
    Dword page_up, page_down, thumb, plus, minus, corner;
    int minimum_thumb_height;
};
struct SliderBar {
    GeneralWidget base;
    Dword minus, plus, fill, thumb, title;
    int position, step, minimum, maximum;
    Dword style;
    Rectangle track;
};
static_assert(sizeof(ScrollBar) == 336 && offsetof(ScrollBar, track) == 288);
static_assert(sizeof(SliderBar) == 324 && offsetof(SliderBar, track) == 304);
struct Frame {
    GeneralWidget base;
    Dword style, left, right, top, bottom, top_left, top_right, bottom_left, bottom_right;
    Dword resize_edges;
    int minimum_height, minimum_width, maximum_height, maximum_width;
};
static_assert(sizeof(Frame) == 320 && offsetof(Frame, resize_edges) == 300);
void layout_scroll_bar(Widget *);
} // namespace nfs::gui::widgets::family
