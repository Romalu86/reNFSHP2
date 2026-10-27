#include "../session/network_lifecycle_runtime.hpp"
#include "nfs/physics/contacts/vehicle_contacts.hpp"
#include "../packets/network_packet_x87.hpp"
#include <cmath>
#include <cstdlib>
namespace nfs::network::session {
namespace {
bits::Stream client_encoder{};
Byte encoder_guard{};
std::int32_t fixed_word(float value) {
    return packet_native::round_fixed_word(value);
}
// RET; no heap owned by this stream.
void finalize_encoder() {
}
} // namespace

void pack_velocity(const float *velocity, void *output) {
    packet_native::pack_velocity(velocity, output);
}

void pack_position(const float *position, void *output) {
    packet_native::pack_position(position, output);
}
// Pack the client snapshot after copying its 20-word state.
void pack_client_car(const void *state, void *output) {
    void *source = const_cast<void *>(state);
    float quaternion[4];
    game::runtime::physics::matrix_to_quaternion(reinterpret_cast<float *>(bytes(source) + 16),
                                                 quaternion);
    at<Dword>(output, 0) = at<Dword>(source, 0);
    for (unsigned axis = 0; axis < 4; ++axis)
        at<std::uint16_t>(output, 26 + axis * 2) =
            std::uint16_t(fixed_word(quaternion[axis]) >> 2);
    pack_position(reinterpret_cast<float *>(bytes(source) + 4), bytes(output) + 4);
    pack_velocity(reinterpret_cast<float *>(bytes(source) + 52), bytes(output) + 14);
    pack_velocity(reinterpret_cast<float *>(bytes(source) + 64), bytes(output) + 20);
    at<std::uint16_t>(output, 34) ^=
        std::uint16_t((at<std::uint16_t>(output, 34) ^ at<std::uint16_t>(source, 76)) & 16383);
}

void send_client_car(void *owner, const void *state) {
    namespace p = packets::pending;
    if (p::compression_enabled() & 255) {
        if (!(encoder_guard & 1)) {
            encoder_guard |= 1;
            bits::construct(&client_encoder);
            std::atexit(finalize_encoder);
        }
        bits::reset(&client_encoder);
        bits::write(&client_encoder, 37, 8);
        bits::write(&client_encoder, Dword(at<std::int16_t>(owner, 376)), 16);
        bits::encode_car(&client_encoder, const_cast<void *>(state));
        const float quotient = float(static_cast<int>(client_encoder.written)) / 8.f;
        packets::send(bytes(owner) + 72, &client_encoder, Dword(std::int64_t(std::ceil(quotient))),
                      4);
    } else {
        alignas(4) Byte packet[40];
        packet[0] = 35;
        at<std::uint16_t>(packet, 2) = at<std::uint16_t>(owner, 376);
        for (unsigned word = 0; word < 9; ++word)
            at<Dword>(packet, 4 + word * 4) = at<Dword>(const_cast<void *>(state), word * 4);
        packets::send(bytes(owner) + 72, packet, sizeof(packet), 4);
    }
}
} // namespace nfs::network::session
