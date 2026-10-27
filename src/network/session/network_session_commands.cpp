#include "../../frontend/network/frontend_network_session.hpp"
#include "network_lifecycle_runtime.hpp"
#include <cstring>
namespace nfs::network {
using namespace packets;

Dword request_client_car(Dword *owner, const ui::CarSettings *settings) {
    if (at<Dword>(owner, 60))
        return 0;
    std::memcpy(bytes(owner) + 436, settings, 84);
    application::pending::LocalFrontendString name(
        reinterpret_cast<gui::String *>(bytes(owner) + 4));
    gui::string_make_narrow(&name.value);
    std::strncpy(reinterpret_cast<char *>(owner) + 460, gui::string_ascii(&name.value), 26);
    const auto phase = at<std::int16_t>(network_game(), 8);
    at<Byte>(owner, 485) = 0;
    at<Dword>(owner, 60) = 1;
    if (phase == 2) {
        alignas(4) Byte packet[88];
        packet[0] = 22;
        at<std::uint16_t>(packet, 2) = at<std::uint16_t>(owner, 376);
        std::memcpy(packet + 4, bytes(owner) + 436, 84);
        at<std::uint16_t>(owner, 380) = 9;
        at<Dword>(owner, 2092) = 0;
        send(bytes(owner) + 72, packet, sizeof(packet));
    }
    return 1;
}

void send_client_vote(Dword *owner, Byte kind, Dword choice) {
    alignas(4) Byte packet[24];
    at<std::uint16_t>(packet, 4) = at<std::uint16_t>(owner, 376);
    packet[6] = kind;
    packet[0] = 43;
    at<Dword>(packet, 8) = choice;
    send(bytes(owner) + 72, packet, sizeof(packet));
}

void send_client_whisper(Dword *owner, const char *text, std::uint16_t recipient) {
    Dword length = Dword(std::strlen(text));
    if (std::int32_t(length) > 128)
        length = 128;
    const Dword size = length + 20;
    auto *packet = static_cast<Byte *>(lifecycle::allocate(size, "ChatMsg", lifecycle::level_pool));
    packet[0] = 33;
    std::strncpy(reinterpret_cast<char *>(packet + 18), text, length);
    const auto sender = at<std::uint16_t>(owner, 376);
    packet[length + 18] = 0;
    at<std::uint16_t>(packet, 2) = sender;
    at<std::uint16_t>(packet, 6) = recipient;
    packet[4] = 1;
    send(bytes(owner) + 72, packet, size);
    gui::application_release(packet);
}

void send_client_rename(Dword *owner, const char *name) {
    alignas(4) Byte packet[28];
    packet[0] = 29;
    std::strncpy(reinterpret_cast<char *>(packet + 4), name, 24);
    at<std::uint16_t>(packet, 2) = at<std::uint16_t>(owner, 376);
    send(bytes(owner) + 72, packet, sizeof(packet));
}
} // namespace nfs::network
