#pragma once
#include "gui_widget_interaction.hpp"
namespace nfs::gui::widgets {
Dword bind_image_resource(ImageResource *, Dword, int);
}
namespace nfs::gui::widgets::family {
struct ImageItem {
    const Method *methods;
    Dword id, enabled, data;
    ImageResource resource;
    int frame;
    Dword reserved;
};
struct ImageList {
    GeneralWidget base;
    ImageSequence sequence;
    Container items;
    Point cell_size, image_size;
    Dword reserved_168, reserved_16c;
    int center_cell, selected;
    Dword timer;
    float speed;
    Dword style;
};
static_assert(sizeof(ImageItem) == 64 && offsetof(ImageItem, frame) == 56);
static_assert(sizeof(ImageList) == 388 && offsetof(ImageList, items) == 320 &&
              offsetof(ImageList, style) == 384);
inline ImageList *image_list(Widget *widget) {
    return reinterpret_cast<ImageList *>(widget);
}
extern Container image_items;
extern Dword image_item_serial;
extern unsigned char image_items_destroyed;
extern const Method image_item_methods[];
int compare_image_item(const void *, const void *);
ImageItem *find_image_item(Dword);
ImageItem *construct_image_item(ImageItem *, int, Dword, Dword);
void destroy_image_item(ImageItem *);
Dword ensure_image_item_frame(ImageItem *, ImageSequence *);
void release_image_item_frame(ImageItem *);
int wrap_image_cell(Widget *, int);
ImageItem *image_item_at(Widget *, int);
Dword image_cell_id(Widget *, int);
Dword image_item_enabled(Dword);
int visible_image_cells(Widget *);
int first_image_cell(Widget *);
int last_image_cell(Widget *);
Dword image_cell_visible(Widget *, int);
void refresh_image_frames(Widget *);
void ensure_image_cell_visible(Widget *, int);
Point *image_scroll_delta(Widget *, Point *, int);
int next_enabled_image_cell(Widget *, int);
int previous_enabled_image_cell(Widget *, int);
Dword select_image_cell(Widget *, int, Dword);
Dword append_image_frame(Widget *, int, Dword, Dword);
Dword append_image_frame_name(Widget *, const char *, Dword, Dword);
} // namespace nfs::gui::widgets::family
