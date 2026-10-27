#pragma once
#include "../network/frontend_network_pages.hpp"
#include "../screens/frontend_menu_controls.hpp"
#include "../../network/gamespy/gamespy_browser.hpp"
#include "../../network/session/network_bandwidth.hpp"
namespace nfs::application::frontend {
struct MultiplayerScreen {
    Widget base;
    Dword reserved_100;
    Widget *back, *retest, *name, *message, *server_count;
    TypeSpinControl *mode;
    int selected_mode;
    Widget *page;
};
struct BrowserView {
    Widget base;
    Dword style, state;
    MultiplayerScreen *owner;
    Widget *host, *join, *refresh;
    Dword reserved_118;
    gui::widgets::family::ListBox *servers;
    Widget *track, *car_class, *dedicated, *bandwidth;
    Widget *sort_buttons[6];
    int maximum_servers, columns[6];
    void *selection;
};
struct PrivateView {
    Widget base;
    Dword style, state;
    MultiplayerScreen *owner;
    Widget *host, *join, *address;
    gui::widgets::family::ListBox *recent;
};
static_assert(sizeof(MultiplayerScreen) == 292 && offsetof(MultiplayerScreen, page) == 288);
static_assert(sizeof(BrowserView) == 360 && offsetof(BrowserView, selection) == 356);
static_assert(sizeof(PrivateView) == 284 && offsetof(PrivateView, recent) == 280);
extern MultiplayerScreen *multiplayer_screen;
extern const std::array<Method, 42> multiplayer_methods;
extern const std::array<Method, 47> browser_view_methods, private_view_methods;
const char *__cdecl multiplayer_class_key();
const char *__cdecl browser_view_class_key();
const char *__cdecl private_view_class_key();
MultiplayerScreen *construct_multiplayer(MultiplayerScreen *, void *);
void destroy_multiplayer(MultiplayerScreen *);
BrowserView *construct_browser_view(BrowserView *, void *);
void destroy_browser_view(BrowserView *);
PrivateView *construct_private_view(PrivateView *, void *);
void destroy_private_view(PrivateView *);
void __fastcall initialize_multiplayer(MultiplayerScreen *, void *);
void __fastcall tick_multiplayer(MultiplayerScreen *, void *, Dword);
void __fastcall leave_multiplayer(MultiplayerScreen *, void *);
Dword __fastcall close_multiplayer(MultiplayerScreen *, void *, Dword, Dword);
void __fastcall multiplayer_child(MultiplayerScreen *, void *, Widget *, Dword, Dword, Dword);
BrowserView *current_browser(MultiplayerScreen *);
void open_server_setup(MultiplayerScreen *);
void change_multiplayer_page(MultiplayerScreen *);
void select_multiplayer_mode(MultiplayerScreen *, int);
void restart_multiplayer_heading(MultiplayerScreen *);
void set_multiplayer_heading(MultiplayerScreen *, Dword);
void finish_browser_view(BrowserView *, Dword);
void finish_private_view(PrivateView *, Dword);
void __fastcall initialize_browser_view(BrowserView *, void *);
void __fastcall tick_browser_view(BrowserView *, void *, Dword);
void __fastcall leave_browser_view(BrowserView *, void *);
void __fastcall browser_view_child(BrowserView *, void *, Widget *, Dword, Dword, Dword);
void connect_selected_server(BrowserView *);
void clear_browser_servers(BrowserView *);
void remove_stale_browser_servers(BrowserView *, Dword);
void clear_browser_details(BrowserView *);
void refresh_browser_details(BrowserView *, void *);
Dword browser_has_room(BrowserView *);
void update_browser_server(BrowserView *, void *, std::uint8_t);
void remove_browser_row(BrowserView *, int);
int find_browser_server_row(BrowserView *, void *);
void *find_browser_server(BrowserView *, const char *, Dword);
int __cdecl compare_server_ping(const void *, const void *, Dword);
void __fastcall initialize_private_view(PrivateView *, void *);
void __fastcall tick_private_view(PrivateView *, void *, Dword);
void __fastcall private_view_child(PrivateView *, void *, Widget *, Dword, Dword, Dword);
void connect_private_server(PrivateView *);
} // namespace nfs::application::frontend
namespace nfs::network::lobby {
namespace sdk = session::pending::browser;
using kernel::Dword;
using Byte = std::uint8_t;
extern sdk::Browser *browser;
extern Dword refresh_started, query_phase, direct_port;
extern char direct_address[500];
extern Byte queued_lan, queued_master, queued_direct, test_master;
extern Byte enforce_version, check_version, request_motd;
extern Byte notify_no_update, fallback_lan;
extern char update_url[2000];
void initialize();
void destroy();
void tick();
void refresh(int);
void cancel();
void clear();
void set_mode(int);
void request_server(void *);
void __cdecl browser_event(sdk::Browser *, Dword, Dword, sdk::Server *, int);
void read_server_basic(void *, sdk::Server **);
void read_server_rules(void *, sdk::Server **);
void set_server_race(void *, const char *);
const char *race_name(int);
Dword server_reversed(void *);
Dword server_mirrored(void *);
void check_for_update(Byte, Byte);
void send_motd();
const char *recent_address(int);
void set_preferred_mode(Dword);
void *client_owner();
void apply_client_name(void *, gui::String);
} // namespace nfs::network::lobby
