#include "movie_yuv.hpp"
namespace nfs::movies::yuv {
#include "movie_yuv_tables.inc"
std::uint64_t multiplier{}, green_mask{}, dither{}, dither_flip{}, component_mask{};
Dword component_order{};
alignas(8) Byte filter_rows[8208]{};

void set_component_order(Dword order) {
    if (component_order == order)
        return;
    // The loop deliberately excludes the final entry255.
    for (unsigned entry = 0; entry < 255; ++entry) {
        auto previous = luminance[entry][0];
        auto next = luminance[entry][1];
        luminance[entry][0] = next;
        next = chroma_u[entry][1];
        luminance[entry][1] = previous;
        previous = chroma_u[entry][0];
        chroma_u[entry][0] = next;
        next = chroma_v[entry][1];
        chroma_u[entry][1] = previous;
        previous = chroma_v[entry][0];
        chroma_v[entry][0] = next;
        chroma_v[entry][1] = previous;
    }
    component_order = order;
}

void configure_packed(Dword format) {
    set_component_order(1);
    dither &= 4294967295u; // The first store clears only the high word.
    if (format) {
        dither = (dither & 18446744069414584320ull) | 263170;
        dither_flip = 1695459815327235ull;
        component_mask = 69806876572254460ull;
        multiplier = 72057628414443528ull;
    } else {
        dither = (dither & 18446744069414584320ull) | 263172;
        dither_flip = 1695472700229126ull;
        component_mask = 141864453447090424ull;
        multiplier = 36028814207221764ull;
    }
    green_mask = 272678883751936ull;
}
} // namespace nfs::movies::yuv
