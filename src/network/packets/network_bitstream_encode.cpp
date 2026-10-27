#include "nfs/network/packets/network_bitstream.hpp"
#include "../../math/precision/extended_float.hpp"
#include "network_packet_x87.hpp"
namespace nfs::network::bits {
// Native FPU status/trap behavior remains separate.
Dword encode_quantized(const Quantizer *quantizer, float input) {
    return packet_native::encode_quantized(quantizer, input);
}

void encode_car(Stream *stream, void *source) {
    write(stream, field<Dword>(source, 0), 32);
    write(stream, field<std::uint8_t>(source, 34) & 1, 1);
    const Dword flags = field<std::uint16_t>(source, 34);
    write(stream, (flags >> 1) & 1, 1);
    write(stream, (flags >> 2) & 1, 1);
    write(stream, (flags >> 4) & 1, 1);
    write(stream, (flags & 224) >> 5, 3);
}

void encode_position(Stream *stream, void *source) {
    write(stream, field<Dword>(source, 0), 32);
    Quantizer *quantizers[] = {&pending::position_x_quantizer, &pending::position_y_quantizer,
                               &pending::position_z_quantizer};
    for (unsigned index = 0; index < 3; ++index) {
        const int width = field<int>(quantizers[index], 300);
        const Dword value = encode_quantized(quantizers[index], field<float>(source, 4 + index * 4));
        write(stream, value, width);
    }
}

void encode_server_car(Stream *stream, void *source) {
    write(stream, 37, 8);
    write(stream, field<std::uint8_t>(source, 1), 8);
    encode_car(stream, static_cast<std::uint8_t *>(source) + 4);
}

void encode_server_position(Stream *stream, void *source) {
    write(stream, 38, 8);
    write(stream, field<std::uint8_t>(source, 1), 8);
    encode_position(stream, static_cast<std::uint8_t *>(source) + 4);
}
} // namespace nfs::network::bits
