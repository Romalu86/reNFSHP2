#include "mad_codec.hpp"
namespace nfs::movies::mad::codec {

void initialize_tables() {
    // The signed loop writes511 entries; clipping entry255 retains its prior representation.
    for (int sample = -256; sample < 255; ++sample) {
        int clamped = sample < -128 ? -128 : sample > 127 ? 127 : sample;
        state<Byte>(8755048 + (Dword(sample) & 511)) = Byte(clamped + 128);
    }
    table(8750696, 0, 15);
    for (Dword index = 1; index < 8; ++index)
        table(8750696, index, 31);
    for (Dword index = 0; index < 8; ++index)
        table(8750728, index, 47);
    for (Dword index = 0; index < 128; ++index)
        table(8751720, index, 63);
    for (const auto &code : short_codewords) {
        Dword repetitions, output, base, index;
        if (code.prefix & 64512) {
            index = Dword(int(code.prefix) >> 7);
            repetitions = shl(1, 9 - code.length);
            output = shl((code.run_level & 64512) | (code.run_level << 16), 6) | code.length;
            base = 8750696;
        } else {
            index = Dword(int(code.prefix) >> 2);
            repetitions = shl(1, 14 - code.length);
            output = shl((code.run_level & 64512) | (code.run_level << 16), 6) | (code.length - 6);
            base = 8753768;
        }
        if (int(repetitions) > 0)
            for (Dword count = 0; count < repetitions; ++count)
                table(base, index + count, output);
    }
    for (const auto &code : long_codewords) {
        Dword length = code.length + 8;
        Dword repetitions, output, base, index;
        if (std::int8_t(code.prefix >> 8) >= 0) {
            index = Dword(int(code.prefix) >> 7);
            repetitions = shl(1, 17 - length);
            output = shl((code.run_level & 64512) | (code.run_level << 16), 6) | (length - 9);
            base = 8752744;
        } else {
            index = Dword(int(code.prefix) >> 10);
            repetitions = shl(1, 14 - length);
            output = shl((code.run_level & 64512) | (code.run_level << 16), 6) | (length - 6);
            base = 8753768;
        }
        if (int(repetitions) > 0)
            for (Dword count = 0; count < repetitions; ++count)
                table(base, index + count, output);
    }
    for (Dword index = 0; index < 32; ++index)
        table(8754792, index, 1);
    Dword value = 4227858432u, index = 0;
    do {
        table(8754920, index, (value + 71303168) | 6);
        table(8754984, index, value | 6);
        value += 4194304;
        ++index;
    } while (int(value) < 0);
    tables_initialized = 1;
}
} // namespace nfs::movies::mad::codec
