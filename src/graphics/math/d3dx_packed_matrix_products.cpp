#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace packed {

float *__stdcall multiply_matrix_rows(float *out, const float *left, const float *right) {
    auto right_first = load(right);
    for (unsigned row = 0; row < 4; ++row) {
        auto input_xy = load(left + row * 4), input_zw = load(left + row * 4 + 2);
        auto x = repeat_low(input_xy), y = repeat_high(input_xy);
        auto z = repeat_low(input_zw), w = repeat_high(input_zw);
        auto xy = _m_pfadd(_m_pfmul(load(right + 4), y), _m_pfmul(right_first, x));
        auto zw = _m_pfadd(_m_pfmul(y, load(right + 6)), _m_pfmul(x, load(right + 2)));
        xy = _m_pfadd(_m_pfmul(load(right + 8), z), xy);
        zw = _m_pfadd(zw, _m_pfmul(z, load(right + 10)));
        xy = _m_pfadd(_m_pfmul(load(right + 12), w), xy);
        zw = _m_pfadd(_m_pfmul(w, load(right + 14)), zw);
        store(out + row * 4, xy);
        if (row != 3)
            right_first = load(right);
        store(out + row * 4 + 2, zw);
    }
    return out;
}
} // namespace packed

void __stdcall multiply_transposed_packed(float *out, const float *left, const float *right) {
    _m_femms();
    __m64 first[4], second[4];
    for (unsigned column = 0; column < 4; ++column) {
        first[column] = _m_punpckldq(low(right + column), low(right + column + 4));
        second[column] = _m_punpckldq(low(right + column + 8), low(right + column + 12));
    }
    float saved[16];
    if (out == left) {
        std::memcpy(saved, left, 64);
        left = saved;
    }
    for (unsigned column = 0; column < 4; ++column) {
        __m64 products[4], tails[4];
        for (unsigned row = 0; row < 4; ++row) {
            products[row] = _m_pfmul(first[column], load(left + row * 4));
            tails[row] = _m_pfmul(second[column], load(left + row * 4 + 2));
        }
        store(out + column * 4,
              _m_pfadd(_m_pfacc(products[0], products[1]), _m_pfacc(tails[0], tails[1])));
        store(out + column * 4 + 2,
              _m_pfadd(_m_pfacc(products[2], products[3]), _m_pfacc(tails[2], tails[3])));
    }
    _m_femms();
}
} // namespace nfs::graphics::d3dx
