#include "nfs/network/packets/network_packet_handlers.hpp"
#include <cmath>
#include <cstdlib>
namespace nfs::network::packets::pending {
namespace {
bits::Stream position_stream{}, car_stream{};
Byte stream_initializers{};
} // namespace
int session_kind{};
Byte compress_internet{}, compress_lan{};

Dword compression_enabled() {
    return (Dword(session_kind) & 4294967040u) | (session_kind ? compress_internet : compress_lan);
}
// Exact C3 entry, no substituted cleanup.
void finalize_position_encoder() {
}
// Exact C3 entry, no substituted cleanup.
void finalize_car_encoder() {
}
// ECX is unused by the entry.
void relay_car_route(void *route, void *descriptor) {
    if (!at<Dword>(route, 12) || at<Dword>(route, 4))
        return;
    void *exclude = at<void *>(descriptor, 16);
    if (exclude && at<std::uint16_t>(exclude, 0) == at<std::uint16_t>(route, 0))
        return;
    const Dword excluded_frame = at<Dword>(descriptor, 20);
    ui::PlayerRecord player;
    ui::initialize_car_settings(&player.car);
    Dword *comparer =
        network_game() ? reinterpret_cast<Dword *>(bytes(player_queue()) + 68) : nullptr;
    if (!queue::copy_matching(player_queue(), address_of(route), address_of(&player), comparer))
        return;
    bool position_only = false;
    if (player.phase == 9) {
        const Dword last = at<Dword>(system(36), 4);
        Dword frame = at<Byte>(&player, 120);
        for (unsigned index = 0; index < 5; ++index) {
            --frame;
            if (static_cast<int>(frame) < 0)
                frame = last;
        }
        position_only = true;
        for (unsigned index = 0; index < 10; ++index) {
            if (frame == excluded_frame) {
                position_only = false;
                break;
            }
            if (static_cast<int>(++frame) > static_cast<int>(last))
                frame = 0;
        }
    }
    void *packet = at<void *>(descriptor, position_only ? 8 : 0);
    Dword size = at<Dword>(descriptor, position_only ? 12 : 4);
    if (compression_enabled() & 255) {
        const Byte mask = position_only ? 2 : 1;
        auto *stream = position_only ? &position_stream : &car_stream;
        if (!(stream_initializers & mask)) {
            stream_initializers |= mask;
            bits::construct(stream);
            std::atexit(position_only ? finalize_position_encoder : finalize_car_encoder);
        }
        bits::reset(stream);
        if (position_only)
            bits::encode_server_position(stream, packet);
        else
            bits::encode_server_car(stream, packet);
        const float quotient = static_cast<float>(static_cast<int>(stream->written)) / 8.0f;
        size = Dword(static_cast<std::int64_t>(std::ceil(quotient)));
        packet = stream;
    }
    send(route, packet, size, 36);
}
} // namespace nfs::network::packets::pending
