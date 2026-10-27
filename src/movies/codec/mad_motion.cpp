#include "mad_codec.hpp"
namespace nfs::movies::mad::codec {

void predict_luma(Dword reference, Dword stride, Dword output, Dword bias) {
    for (Dword row = 0; row < 8; ++row) {
        for (Dword column = 0; column < 8; ++column) {
            Dword sample = load<Byte>(address(reference + column));
            store(address(output + column * 4), (sample + bias) << 16);
        }
        reference += stride;
        output += 64;
    }
}

void predict_chroma(Dword reference, Dword stride, Dword output, Dword bias) {
    for (Dword row = 0; row < 8; ++row) {
        for (Dword column = 0; column < 8; ++column) {
            Dword sample = load<Byte>(address(reference + column));
            store(address(output + column * 4), (sample + bias) << 16);
        }
        reference += stride;
        output += 32;
    }
}

void write_luma(Dword source, Dword output, Dword stride) {
    for (Dword row = 0; row < 16; ++row) {
        for (Dword column = 0; column < 16; ++column) {
            Dword sample = load<std::uint16_t>(address(source + column * 4 + 2)) & 511;
            store(address(output + column), state<Byte>(8755048 + sample));
        }
        source += 64;
        output += stride;
    }
}

void write_chroma(Dword source, Dword output, Dword stride) {
    for (Dword row = 0; row < 8; ++row) {
        for (Dword column = 0; column < 8; ++column) {
            Dword sample = load<std::uint16_t>(address(source + column * 4 + 2)) & 511;
            store(address(output + column), state<Byte>(8755048 + sample));
        }
        source += 32;
        output += stride;
    }
}
} // namespace nfs::movies::mad::codec
