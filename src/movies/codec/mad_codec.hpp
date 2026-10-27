#pragma once
#include "../playback/movie_mad.hpp"
#include <cstring>
namespace nfs::movies::mad::codec {
struct Codeword {
    Dword length, run_level, reserved, prefix;
};
static_assert(sizeof(Codeword) == 16);
extern const Codeword short_codewords[94], long_codewords[128];
extern const Dword quantization_curve[63];
extern Byte decoder_storage[6692], transform_storage[1104];
template <class T> T load(const void *source) {
    T value;
    std::memcpy(&value, source, sizeof(T));
    return value;
}
template <class T> void store(void *destination, T value) {
    std::memcpy(destination, &value, sizeof(T));
}
template <class T = Byte> T *address(Dword value) {
    return reinterpret_cast<T *>(value);
}
template <class T> Dword address(T *value) {
    return reinterpret_cast<Dword>(value);
}
inline Dword word(unsigned va) {
    return load<Dword>(transform_storage + va - 7147552);
}
inline void word(unsigned va, Dword value) {
    store(transform_storage + va - 7147552, value);
}
inline float real(unsigned va) {
    return load<float>(transform_storage + va - 7147552);
}
inline Dword storage(unsigned va) {
    return address(decoder_storage) + va - 8748872;
}
template <class T> T &state(unsigned va) {
    return *address<T>(storage(va));
}
extern Dword &bit_window;
extern int &available_bits;
extern Byte *&bit_cursor;
extern Dword &predicted;
extern Dword &tables_initialized;
inline Dword table(unsigned va, Dword index) {
    return load<Dword>(address(storage(va) + index * 4));
}
inline void table(unsigned va, Dword index, Dword value) {
    store(address(storage(va) + index * 4), value);
}
inline Dword shl(Dword value, Dword count) {
    return value << (count & 31);
}
inline Dword signed_high_product(Dword first, Dword second) {
    return Dword(std::uint64_t(std::int64_t(std::int32_t(first)) * std::int32_t(second)) >> 32);
}
inline Dword scaled_product(Dword first, Dword second) {
    const auto bits = std::uint64_t(std::int64_t(std::int32_t(first)) * std::int32_t(second));
    return Dword(bits >> 16) + Dword((bits >> 15) & 1);
}
void initialize_tables();
void prepare_frame(const Byte *, Dword, int);
void skip_bits(Dword);
int motion_value();
int decode_coefficients();
void fill_dc(Dword, Dword);
void predict_luma(Dword, Dword, Dword, Dword);
void predict_chroma(Dword, Dword, Dword, Dword);
void write_luma(Dword, Dword, Dword);
void write_chroma(Dword, Dword, Dword);
void inverse_columns(Dword);
void inverse_rows(Dword);
void inverse_transform(Dword, Dword);
void decode_macroblock(Dword, Dword, Dword, Dword, Dword, Dword, Dword);
Dword change_fp_control(Dword, Dword);
} // namespace nfs::movies::mad::codec
