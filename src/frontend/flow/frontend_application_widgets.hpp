#pragma once
#include "../../gui/widgets/gui_widget_family.hpp"
#include <algorithm>
#include <array>
namespace nfs::gui::widgets {
const char *__cdecl base_class_name();
const char *__cdecl general_class_name();
const char *__cdecl interface_class_name();
void destroy_interface(Widget *);
void __fastcall request_layout(Widget *, void *);
engine::Rectangle *client_bounds(Widget *, engine::Rectangle *);
namespace pending {
void *construct_interface(void *, void *);
void *construct_dialog(void *, void *);
} // namespace pending
} // namespace nfs::gui::widgets
namespace nfs::gui::widgets::family {
void finish_modal(Widget *, Dword, Dword);
void move_to_first_draw_position(Widget *);
Widget *named_sibling(Widget *, const String *);
} // namespace nfs::gui::widgets::family
namespace nfs::gui {
Dword create_widget_timer(Dword, float);
void remove_widget_timer(Dword);
void list_clear(List *);
} // namespace nfs::gui
namespace nfs::application::pending {
void *find_child(void *, const char *);
void set_widget_visible(void *, gui::Dword);
gui::widgets::Widget *widget_root(gui::widgets::Widget *);
} // namespace nfs::application::pending
namespace nfs::application::frontend {
using gui::Dword;
using gui::widgets::GeneralWidget;
using gui::widgets::Method;
using gui::widgets::Widget;
struct Ticker {
    GeneralWidget base;
    gui::List pending, active;
    int maximum_messages;
    float accumulated_seconds;
    int spacing;
    float speed;
    Dword timer_id;
};
struct TickerMessage {
    const gui::Destroy *methods;
    gui::engine::Point position;
    int elapsed, duration;
    gui::String text;
    Dword flags, reserved, initial_colour, colour, id;
    gui::engine::Rectangle text_bounds;
    int stationary_ticks;
    std::uint8_t moved, padding[3];
};
static_assert(sizeof(Ticker) == 332 && offsetof(Ticker, active) == 288);
static_assert(sizeof(TickerMessage) == 92 && offsetof(TickerMessage, text) == 24);
const char *__cdecl ticker_class_key();
Dword queue_ticker_message(Ticker *, const gui::String *, Dword, const Dword *, Dword, float, Dword);
void clear_ticker_messages(Ticker *);
extern const std::array<Method, 47> ticker_methods;
extern const gui::Destroy ticker_message_methods[];
extern gui::Container ticker_messages;
extern Dword ticker_message_serial;
void register_main_menu_callbacks();
void initialize_ticker_message(TickerMessage *, void *, const gui::engine::Rectangle *);
void advance_ticker_message(TickerMessage *, int, const gui::engine::Rectangle *);
void begin_next_ticker_message(Ticker *);
void recycle_ticker_message(Ticker *);
Dword ticker_is_full(Ticker *);
Dword cycle_ticker_hue(Dword, int);
void set_ticker_message_duration(Dword, float);
void expire_ticker_message(Dword);
} // namespace nfs::application::frontend
