#pragma once
#include "../screens/frontend_dialogs.hpp"
#include "../flow/frontend_toggle_widget.hpp"
#include "../flow/frontend_scene_widgets.hpp"
#include "../../gui/widgets/gui_widget_list.hpp"
#include "../../gui/widgets/gui_widget_list_sort.hpp"
#include "nfs/network/session/network_gui.hpp"
#include "nfs/network/session/network_player_state.hpp"
namespace nfs::profiles {
gui::widgets::family::ListItem *construct_profile_list_item(gui::widgets::family::ListItem *);
}
namespace nfs::application::frontend {
struct NetworkOptions {
    Widget base;
    alignas(4) std::uint8_t dialog_state[32];
    Widget *server, *car, *hud, *audio, *disconnect, *close;
};
struct ClientCarSelection {
    Widget base;
    alignas(4) std::uint8_t dialog_state[32];
    Widget *select, *cancel, *observe;
    ProfileToggle *transmission;
    ColourWheel *colours;
    gui::widgets::family::ListBox *cars;
};
struct NetworkStatistics {
    Widget base;
    alignas(4) std::uint8_t dialog_state[32];
    const int *columns;
    int column, next_column;
    Dword rotation_deadline;
    Widget *phase, *status, *rotation, *race_type, *server_name, *car_class, *track, *direction;
    gui::widgets::family::ListBox *players;
};
struct ServerSetup {
    Widget base;
    alignas(4) std::uint8_t dialog_state[32];
    Widget *accept, *cancel, *dedicated;
    ProfileToggle *track, *direction, *race_type, *car_class;
    Widget *name, *password;
    ProfileToggle *laps, *maximum_clients, *collisions;
};
static_assert(sizeof(NetworkOptions) == 312 && sizeof(ClientCarSelection) == 312);
static_assert(sizeof(NetworkStatistics) == 340 && offsetof(NetworkStatistics, players) == 336);
static_assert(sizeof(ServerSetup) == 336 && offsetof(ServerSetup, laps) == 324);
extern const std::array<Method, 49> network_options_methods, client_car_selection_methods,
    network_statistics_methods, server_setup_methods;
extern std::uint8_t &network_statistics_requested;    
extern network::players::Visitor statistics_visitor;
const char *__cdecl network_options_class_key();
const char *__cdecl client_car_selection_class_key();
const char *__cdecl network_statistics_class_key();
const char *__cdecl server_setup_class_key();
void __fastcall initialize_network_options(NetworkOptions *, void *);
void __fastcall tick_network_options(NetworkOptions *, void *, Dword);
void __fastcall network_options_child(NetworkOptions *, void *, Widget *, Dword, Dword, Dword);
void __fastcall leave_network_options(NetworkOptions *, void *);
Dword __fastcall close_network_options(NetworkOptions *, void *, Dword, Dword);
void __fastcall initialize_client_car_selection(ClientCarSelection *, void *);
void __fastcall tick_client_car_selection(ClientCarSelection *, void *, Dword);
void __fastcall client_car_selection_child(ClientCarSelection *, void *, Widget *, Dword, Dword,
                                           Dword);
Dword __fastcall close_client_car_selection(ClientCarSelection *, void *, Dword, Dword);
void refresh_client_car_colours(ClientCarSelection *, Dword);
void __fastcall initialize_network_statistics(NetworkStatistics *, void *);
void __fastcall tick_network_statistics(NetworkStatistics *, void *, Dword);
void __fastcall network_statistics_child(NetworkStatistics *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall network_statistics_key(NetworkStatistics *, void *, Dword, Dword);
void __fastcall clear_network_statistics_request(NetworkStatistics *, void *);
void rotate_network_statistic(NetworkStatistics *);
void format_network_statistic(const network::ui::PlayerRecord *, int, gui::String *);
Dword format_network_position(int, gui::String *, int *);
Dword __fastcall visit_network_statistics(void *, void *, network::ui::PlayerRecord *, Dword, Dword,
                                         NetworkStatistics *);
int __cdecl compare_network_positions(const void *, const void *, Dword);
void refresh_network_statistics_rows(NetworkStatistics *);
void __fastcall initialize_server_setup(ServerSetup *, void *);
void __fastcall tick_server_setup(ServerSetup *, void *, Dword);
void __fastcall server_setup_child(ServerSetup *, void *, Widget *, Dword, Dword, Dword);
Dword __fastcall close_server_setup(ServerSetup *, void *, Dword, Dword);
Widget *network_page_child(void *, const char *, gui::widgets::TypeName);
inline void network_reference(Widget *widget, Dword reference) {
    pending::LocalFrontendString text;
    gui::string_reference(&text.value, reference);
    game::runtime::invoke(widget, 7, &text.value);
}
struct NetworkListItem {
    gui::widgets::family::ListItem value;
    explicit NetworkListItem(const gui::String *);
    NetworkListItem(const gui::String *, Dword);
    ~NetworkListItem();
};
gui::widgets::family::ListItem *construct_ascii_item(gui::widgets::family::ListItem *,const char *,Dword);
} // namespace nfs::application::frontend
namespace nfs::network::session {
void request_spectator_mode(void *);
void send_client_selection(void *);
} // namespace nfs::network::session
