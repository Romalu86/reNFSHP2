#include "nfs/network/packets/network_bitstream.hpp"
#include "../../math/precision/extended_float.hpp"
#include "network_packet_x87.hpp"
namespace nfs::network::bits {

kernel::numeric::Extended retained_decode_quantized(const Quantizer *quantizer, Dword value) {
    unsigned char native[10];
    packet_native::decode_quantized_native80(quantizer, value, native);
    return kernel::numeric::Extended::from_native80(native, kernel::numeric::current_format());
}
// Compatibility return for unreviewed users of the public scalar ABI.
long double decode_quantized(const Quantizer *quantizer, Dword value) {
    return double(retained_decode_quantized(quantizer, value));
}
// Bits 3 and 8..15 retain their incoming values.
void decode_car(Stream *stream, void *destination) {
    read(stream, destination, 32);
    Dword value = 0;
    read(stream, &value, 1);
    Dword flags = (field<std::uint16_t>(destination, 34) & ~1u) | Dword(value == 1);
    field<std::uint16_t>(destination, 34) = std::uint16_t(flags);
    const unsigned shifts[] = {1, 2, 4};
    for (unsigned shift : shifts) {
        value = 0;
        read(stream, &value, 1);
        flags = (flags & ~(1u << shift)) ^ (Dword(value == 1) << shift);
        field<std::uint16_t>(destination, 34) = std::uint16_t(flags);
    }
    value = 0;
    read(stream, &value, 3);
    flags ^= std::uint16_t((flags ^ (value << 5)) & 224);
    field<std::uint16_t>(destination, 34) = std::uint16_t(flags);
}

void decode_position(Stream *stream, void *destination) {
    packet_native::decode_position(stream, destination);
}

void decode_client_car(Stream *stream, void *destination) {
    auto *packet = static_cast<std::uint8_t *>(destination);
    read(stream, packet, 8);
    read(stream, packet + 2, 16);
    decode_car(stream, packet + 4);
    packet[0] = 35;
}

void decode_server_car(Stream *stream, void *destination) {
    auto *packet = static_cast<std::uint8_t *>(destination);
    read(stream, packet, 8);
    read(stream, packet + 1, 8);
    decode_car(stream, packet + 4);
    packet[0] = 35;
}

void decode_server_position(Stream *stream, void *destination) {
    auto *packet = static_cast<std::uint8_t *>(destination);
    read(stream, packet, 8);
    read(stream, packet + 1, 8);
    decode_position(stream, packet + 4);
    packet[0] = 36;
}
} // namespace nfs::network::bits
