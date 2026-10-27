#include "mad_codec.hpp"
namespace nfs::movies::mad::codec {

void prepare_frame(const Byte *source, Dword use_reference, int quantizer) {
    if (!tables_initialized)
        initialize_tables();
    bit_window = (Dword(load<std::uint16_t>(source)) << 16) | load<std::uint16_t>(source + 2);
    available_bits = 32;
    bit_cursor = const_cast<Byte *>(source + 4);
    predicted = use_reference;
    table(8748880, 0, scaled_product(524288, word(7148140)));
    for (Dword index = 0; index < 63; ++index) {
        Dword amplitude = shl(quantization_curve[index] * Dword(quantizer), 13);
        table(8748884, index, scaled_product(amplitude, word(7148144 + index * 4)));
    }
}

void skip_bits(Dword count) {
    bit_window = shl(bit_window, count);
    available_bits = int(Dword(available_bits) - count);
    if (available_bits < 16) {
        Dword shift = 16 - Dword(available_bits);
        Dword next = load<std::uint16_t>(bit_cursor);
        available_bits = int(Dword(available_bits) + 16);
        bit_window |= shl(next, shift);
        bit_cursor = address(address(bit_cursor) + 2);
    }
}

int motion_value() {
    Dword entry = table(8754792, bit_window >> 26);
    skip_bits(entry & 255);
    return int(entry) >> 22;
}
} // namespace nfs::movies::mad::codec
