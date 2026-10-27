#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/core/configuration/launch_options.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include "../../gui/lifecycle/gui_input.hpp"

namespace nfs::services::pending {
Dword &server_mode = *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + 88);
Dword &join_mode = *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + 96);
Dword &dedicated_server =
    *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + 104);
Dword &player_name_requested =
    *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + 116);
Dword &port_requested = *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + 124);
Dword &car_override = *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + 48);
Dword &opponents_override =
    *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + 152);
char (&player_name)[24] =
    *reinterpret_cast<char (*)[24]>(reinterpret_cast<Byte *>(&launch::options) + 1004);
char (&join_address)[24] =
    *reinterpret_cast<char (*)[24]>(reinterpret_cast<Byte *>(&launch::options) + 884);
char (&port_text)[24] = *reinterpret_cast<char (*)[24]>(reinterpret_cast<Byte *>(&launch::options) +
                                                        1052);
char (&car_name)[24] = *reinterpret_cast<char (*)[24]>(reinterpret_cast<Byte *>(&launch::options) +
                                                       596);
char (&opponents_text)[24] =
    *reinterpret_cast<char (*)[24]>(reinterpret_cast<Byte *>(&launch::options) + 1220);
namespace {
Dword network_kind_value{};
void *network_message_object{};
void *gui_archive_object{};
HWND console_window_value{};
HHOOK console_hook_value{};
HINSTANCE application_instance_value{};
alignas(4) Byte rumble_data[48] = {
#include "../../input/feedback/service_lifecycle_rumble_data.inc"
};
} // namespace
Dword &network_kind = network_kind_value;
void *&network_messages = network_message_object;
void *&gui_archive = gui_archive_object;
Dword &gui_repeat_interval = gui::input::repeat_interval,
     &gui_repeat_delay = gui::input::initial_repeat_delay;
HWND &console_window = console_window_value;
HHOOK &console_hook = console_hook_value;
HINSTANCE &application_instance = application_instance_value;
float *const rumble_values = reinterpret_cast<float *>(rumble_data);
Byte *const surface_feedback = game::runtime::saved::surface_parameters + 12;
} // namespace nfs::services::pending
