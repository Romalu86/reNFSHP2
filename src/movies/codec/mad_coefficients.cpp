#include "mad_codec.hpp"
namespace nfs::movies::mad::codec {

int decode_coefficients() {
    Byte *cursor = bit_cursor;
    Dword window = bit_window;
    int bits = available_bits;
    word(7147884, Dword(int(window) >> 24) * table(8748880, 0));
    auto consume = [&](Dword count) {
        window = shl(window, count);
        bits = int(Dword(bits) - count);
        if (bits < 16) {
            Dword next = load<std::uint16_t>(cursor);
            cursor = address(address(cursor) + 2);
            window |= shl(next, 16 - Dword(bits));
            bits = int(Dword(bits) + 16);
        }
    };
    consume(8);
    // Seven groups of nine words, excluding the saved DC coefficient.
    for (Dword index = 0; index < 63; ++index)
        word(7147888 + index * 4, 0);
    Dword coefficient = 1;
    for (;;) {
        Dword entry = table(8750696, window >> 23);
        Byte count = Byte(entry);
        if (std::int8_t(count) > 9) {
            if (count < 32) {
                if (count < 16) {
                    consume(9);
                    entry = table(8752744, window >> 24);
                } else {
                    consume(6);
                    entry = table(8753768, window >> 24);
                }
            } else if (count < 48) {
                consume(6);
                entry = (window & 4294901760u) | 16;
            } else {
                consume(2);
                bit_cursor = cursor;
                bit_window = window;
                available_bits = bits;
                return int(coefficient);
            }
        }
        Dword run = (entry >> 16) & 63;
        consume(entry & 255);
        coefficient += run;
        Dword offset = word(7148400 + coefficient * 4);
        Dword level = Dword(int(entry) >> 22);
        ++coefficient;
        word(7147884 + offset, level * load<Dword>(address(storage(8748880) + offset)));
    }
}
} // namespace nfs::movies::mad::codec
