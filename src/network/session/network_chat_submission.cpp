#include "network_chat_submission.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include <cstring>

namespace nfs::services {
kernel::Dword pop_console_message(void *, void *); // Shared chat queue owner.
}
namespace nfs::network {

void send_client_chat(Dword *client, const char *text) {
    Dword length = static_cast<Dword>(std::strlen(text));
    if (static_cast<std::int32_t>(length) > 128)
        length = 128;
    const Dword packet_bytes = length + 12;
    auto *packet = static_cast<std::uint8_t *>(
        lifecycle::allocate(packet_bytes, "ChatMsg", lifecycle::level_pool));
    packet[0] = 24;
    std::strncpy(reinterpret_cast<char *>(packet + 8), text, length);
    const auto sender = field<std::uint16_t>(client, 376);
    packet[length + 8] = 0;
    field<Dword>(packet, 4) = 0;
    field<std::uint16_t>(packet, 2) = sender;
    send_packet(reinterpret_cast<Dword>(reinterpret_cast<std::uint8_t *>(client) + 72),
                reinterpret_cast<Dword>(packet), packet_bytes, 0);
    gui::application_release(packet);
}


void send_client_wide_chat(Dword *client, const wchar_t *text, int length) {
    Dword text_bytes = Dword(length) * 2;
    if (static_cast<std::int32_t>(text_bytes) > 256)
        text_bytes = 256;
    const Dword packet_bytes = text_bytes + 13;
    auto *packet = static_cast<std::uint8_t *>(
        lifecycle::allocate(packet_bytes, "WideChatMsg", lifecycle::level_pool));
    packet[0] = 50;
    std::memcpy(packet + 8, text, text_bytes);
    field<std::uint16_t>(packet, (static_cast<std::int32_t>(text_bytes) / 2) * 2 + 8) = 0;
    const auto sender = field<std::uint16_t>(client, 376);
    field<Dword>(packet, 4) = 0;
    field<std::uint16_t>(packet, 2) = sender;
    send_packet(reinterpret_cast<Dword>(reinterpret_cast<std::uint8_t *>(client) + 72),
                reinterpret_cast<Dword>(packet), packet_bytes, 0);
    gui::application_release(packet);
}

namespace ui {

std::uint8_t read_console_input(char *output, Dword capacity) {
    void *queue = lifecycle::runtime::console_objects[2];
    if ((queue || output) && capacity) {
        ConsoleText message;
        message.methods = console_methods;
        if (services::pop_console_message(queue, &message)) {
            std::strncpy(output, message.text, capacity);
            return 1;
        }
    }
    return 0;
}
} // namespace ui
} // namespace nfs::network
