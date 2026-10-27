#include "mad_codec.hpp"
namespace nfs::movies::mad::codec {

void decode_macroblock(Dword reference_y, Dword reference_u, Dword reference_v, Dword output_y,
                       Dword output_u, Dword output_v, Dword stride) {
    Dword chroma_stride = Dword(int(stride) >> 1);
    Dword predicted_blocks = 0;
    if (predicted) {
        if ((bit_window & 3221225472u) == 0)
            skip_bits(2);
        else {
            if (int(bit_window) < 0) {
                predicted_blocks = 1023;
                skip_bits(1);
            } else {
                predicted_blocks = bit_window >> 24;
                skip_bits(8);
            }
            int horizontal = motion_value();
            int vertical = motion_value();
            reference_y += Dword(vertical) * stride + Dword(horizontal);
            Dword chroma_offset = Dword(vertical >> 1) * chroma_stride + Dword(horizontal >> 1);
            reference_u += chroma_offset;
            reference_v += chroma_offset;
        }
    }
    auto decode_luma = [&](Dword mask, Dword destination, Dword reference) {
        if (!(predicted_blocks & mask)) {
            int count = decode_coefficients();
            if (count == 1)
                fill_dc(destination, 16);
            else
                inverse_transform(destination, 16);
        } else {
            Dword bias = Dword(motion_value()) * 2 - 128;
            predict_luma(reference, stride, destination, bias);
        }
    };
    decode_luma(1, storage(8749664), reference_y);
    decode_luma(2, storage(8749696), reference_y + 8);
    decode_luma(4, storage(8750176), reference_y + stride * 8);
    decode_luma(8, storage(8750208), reference_y + stride * 8 + 8);
    auto decode_chroma = [&](Dword mask, Dword destination, Dword reference) {
        if (!(predicted_blocks & mask)) {
            int count = decode_coefficients();
            if (count == 1)
                fill_dc(destination, 8);
            else
                inverse_transform(destination, 8);
        } else {
            Dword bias = Dword(motion_value()) * 2 - 128;
            predict_chroma(reference, chroma_stride, destination, bias);
        }
    };
    decode_chroma(16, storage(8749144), reference_u);
    decode_chroma(32, storage(8749400), reference_v);
    write_luma(storage(8749664), output_y, stride);
    write_chroma(storage(8749144), output_u, chroma_stride);
    write_chroma(storage(8749400), output_v, chroma_stride);
}
} // namespace nfs::movies::mad::codec
