#pragma once
#include "nfs/network/session/network_gui.hpp"

namespace nfs::network {
void send_client_chat(Dword *client, const char *text);
void send_client_wide_chat(Dword *client, const wchar_t *text, int length);
namespace ui {
std::uint8_t read_console_input(char *output, Dword capacity);
}
} // namespace nfs::network
