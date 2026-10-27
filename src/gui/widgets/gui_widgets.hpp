#pragma once
#include "nfs/gui/engine/gui_engine.hpp"
namespace nfs::gui::widgets {
using Method = engine::Method;
using engine::Point;
using engine::Rectangle;
struct Widget {
    const Method *methods;
    Dword id;
    Container children, draw_order, tab_order;
    Widget *parent;
    Dword flags;
    int z_order, tab_index;
    String name;
    PoolString *resource;
    Rectangle bounds;
    Point scroll;
    Dword layout_flags, value_84;
    Dword colour;
    String text;
    Dword font_style, palette;
    Rectangle clip, padding;
    Dword value_d0;
    Container *handlers;
    Dword value_d8, modal_result, value_e0;
    String tooltip;
    Dword timer, options;
};
struct GeneralWidget {
    Widget base;
    Dword style, state;
};
struct ImageResource {
    const Method *methods;
    Dword image;
    Rectangle bounds;
    Point origin;
};
struct SimpleImage {
    Widget base;
    ImageResource resource;
};
static_assert(sizeof(Widget) == 256 && offsetof(Widget, resource) == 92);
static_assert(sizeof(GeneralWidget) == 264 && sizeof(SimpleImage) == 296);
static_assert(sizeof(ImageResource) == 40 && offsetof(Widget, tooltip) == 228);
using Create = void *(__cdecl *)(void *);
struct ClassRecord {
    const char *name, *base_name;
    Create create;
    Dword hash;
    ClassRecord *next;
};
static_assert(sizeof(ClassRecord) == 20);
extern ClassRecord *classes;
extern Dword active_widget_id;
extern const Method widget_methods[], general_methods[], simple_image_methods[],
    image_resource_methods[];
ClassRecord *register_class(ClassRecord *, const char *, Create, const char *);
void unregister_class(ClassRecord *);
void *construct_widget(void *, void *);
void *construct_simple_image(void *, void *);
void *construct_base(void *, void *);
void destroy_base(Widget *);
void destroy_general(GeneralWidget *);
int compare_order(const void *, const void *);
int compare_tab(const void *, const void *);
int compare_id(const void *, const void *);
Widget *child_at(Widget *, int, Dword);
Widget *find_child(Widget *, Dword);
Dword enabled_in_ancestors(Widget *);
void set_enabled(Widget *, Dword);
Dword has_ancestor(Widget *, const void *);
Dword may_activate(Widget *);
Dword dispatch_target(Dword, Dword, Dword, Dword);
void clear_active(Widget *);
Widget *activate(Widget *);
Dword activate_first_child(Widget *);
Dword activate_last_child(Widget *);
Dword inherited_font_style(Widget *);
extern const Method interface_methods[], simple_text_methods[];
void capture_widget(Widget *);
void release_capture(Widget *);
Dword contains_modal(Widget *);
Dword hit_branch(Widget *);
Dword next_pointer_target(Widget *);
Dword contains_point(const Rectangle *, const Point *);
namespace pending {
Dword __fastcall load_configuration(Widget *, void *, void *);
Dword __fastcall save_configuration(Widget *, void *, void *);
void __fastcall layout(Widget *, void *, Dword);
Dword __fastcall hit_test(Widget *, void *);
Dword __fastcall navigate(Widget *, void *, Dword, Dword, Dword);
Dword __fastcall enter_modal(Widget *, void *);
void __fastcall leave_modal(Widget *, void *);
Dword __fastcall handle_event(Widget *, void *, Dword, Dword, Dword);
Point *pointer_position();
void next_tab(Widget *);
void previous_tab(Widget *);
} // namespace pending
Dword attach_child(Widget *, Widget *);
void *descendant(Widget *, Dword);
ImageResource *construct_image_resource(ImageResource *);
void destroy_image_resource(ImageResource *);
Dword load_image_resource(ImageResource *, const String *);
void clear_image_resource(ImageResource *);
ImageResource *copy_image_resource(ImageResource *, const ImageResource *);
Dword register_widget(ContextState *, Widget *);
Dword unregister_widget(ContextState *, Widget *);
} // namespace nfs::gui::widgets
