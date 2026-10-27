#pragma once
#include "gui_widget_resources.hpp"
namespace nfs::gui::widgets::family {
extern Point default_size;
void construct_size(Point *);
Point *assign_size(Point *, Point *, const Point *);
Dword read_size(void *, const char *, Point *, const Point *);
Dword write_size(void *, const char *, const Point *, const Point *);
} // namespace nfs::gui::widgets::family
