#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;

void __stdcall translation_matrix_packed(float *out, float x, float y, float z) {
    _m_femms();
    auto zero = _m_pxor(_m_from_int(0), _m_from_int(0));
    auto one_low = bits(0, 1065353216), one_high = bits(1065353216, 0);
    store(out, one_low);
    store(out + 2, zero);
    store(out + 4, one_high);
    store(out + 6, zero);
    store(out + 8, zero);
    store(out + 10, one_low);
    store(out + 12, _m_punpckldq(low(&x), low(&y)));
    store(out + 14, _m_por(low(&z), one_high));
    _m_femms();
}

void __stdcall scaling_matrix_packed(float *out, float x, float y, float z) {
    _m_femms();
    auto zero = _m_from_int(0);
    store(out, low(&x));
    store(out + 2, zero);
    store(out + 6, zero);
    store(out + 8, zero);
    store(out + 12, zero);
    store(out + 4, _m_punpckldq(zero, low(&y)));
    store(out + 10, low(&z));
    store(out + 14, bits(1065353216, 0));
    _m_femms();
}

void __stdcall transpose_packed(float *out, const float *input) {
    auto a = load(input), b = load(input + 4), c = load(input + 10), d = load(input + 14);
    store(out, _m_punpckldq(a, b));
    store(out + 4, _m_punpckhdq(a, b));
    a = load(input + 2);
    b = load(input + 6);
    auto e = load(input + 8);
    store(out + 10, _m_punpckldq(c, d));
    store(out + 14, _m_punpckhdq(c, d));
    c = load(input + 12);
    store(out + 8, _m_punpckldq(a, b));
    store(out + 12, _m_punpckhdq(a, b));
    store(out + 6, _m_punpckhdq(e, c));
    store(out + 2, _m_punpckldq(e, c));
    _m_femms();
}
} // namespace nfs::graphics::d3dx
