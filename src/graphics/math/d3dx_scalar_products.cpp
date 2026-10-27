#include "d3dx_dispatch_kernels.hpp"
#include <cstring>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Scalar products require the retained x87 compiler.");

namespace nfs::graphics::d3dx {
namespace {
void store_result_word(volatile Dword &word, long double value) {
    const float rounded = static_cast<float>(value);
    Dword bits;
    std::memcpy(&bits, &rounded, sizeof(bits));
    word = bits;
}

void copy_result(float *output, const volatile Dword *words) {
    for (unsigned axis = 0; axis < 4; ++axis) {
        const Dword bits = words[axis];
        std::memcpy(output + axis, &bits, sizeof(bits));
    }
}
} // namespace

void __stdcall cross_vector4_scalar(float *output, const float *first, const float *second,
                                    const float *third) {
    const volatile float *a = first;
    const volatile float *b = second;
    const volatile float *c = third;
    long double zw_first = static_cast<long double>(c[3]);
    zw_first *= b[2];
    long double zw_second = static_cast<long double>(b[3]);
    zw_second *= c[2];
    long double yw_first = static_cast<long double>(c[3]);
    yw_first *= b[1];
    long double yw_second = static_cast<long double>(b[3]);
    yw_second *= c[1];
    volatile float yw = static_cast<float>(yw_first - yw_second);
    volatile float yz_first = static_cast<float>(static_cast<long double>(b[1]) * c[2]);
    volatile float yz_second = static_cast<float>(static_cast<long double>(b[2]) * c[1]);

    // Keep the completed FLOAT32 results as raw DWORDs for the forward copy.
    volatile Dword result[4];
    long double value = zw_first - zw_second;
    value *= a[1];
    long double term = static_cast<long double>(yw) * a[2];
    value -= term;
    term = static_cast<long double>(yz_first) - yz_second;
    term *= a[3];
    value += term;
    store_result_word(result[0], value);

    volatile float xw_first = static_cast<float>(static_cast<long double>(b[3]) * c[0]);
    volatile float xw_second = static_cast<float>(static_cast<long double>(c[3]) * b[0]);
    long double xz_first = static_cast<long double>(c[0]) * b[2];
    long double xz_second = static_cast<long double>(b[0]) * c[2];
    volatile float xz = static_cast<float>(xz_first - xz_second);
    value = zw_second - zw_first;
    value *= a[0];
    term = static_cast<long double>(xw_first) - xw_second;
    term *= a[2];
    value -= term;
    term = static_cast<long double>(xz) * a[3];
    value += term;
    store_result_word(result[1], value);

    long double xy_first = static_cast<long double>(b[0]) * c[1];
    long double xy_second = static_cast<long double>(c[0]) * b[1];
    value = static_cast<long double>(yw) * a[0];
    term = static_cast<long double>(xw_second) - xw_first;
    term *= a[1];
    value -= term;
    term = xy_first - xy_second;
    term *= a[3];
    value += term;
    store_result_word(result[2], value);

    value = static_cast<long double>(yz_second) - yz_first;
    value *= a[0];
    term = static_cast<long double>(xz) * a[1];
    value -= term;
    term = xy_second - xy_first;
    term *= a[2];
    value += term;
    store_result_word(result[3], value);
    copy_result(output, result);
}

void __stdcall transform_plane_scalar(float *output, const float *plane, const float *matrix) {
    const volatile float *source = plane;
    const volatile float *transform = matrix;
    volatile Dword result[4];

    long double value = static_cast<long double>(transform[8]) * source[2];
    long double term = static_cast<long double>(transform[12]) * source[3];
    value += term;
    term = static_cast<long double>(transform[4]) * source[1];
    value += term;
    term = static_cast<long double>(source[0]) * transform[0];
    value += term;
    store_result_word(result[0], value);
    for (unsigned axis = 1; axis < 4; ++axis) {
        value = static_cast<long double>(transform[8 + axis]) * source[2];
        term = static_cast<long double>(transform[axis]) * source[0];
        value += term;
        term = static_cast<long double>(transform[12 + axis]) * source[3];
        value += term;
        term = static_cast<long double>(transform[4 + axis]) * source[1];
        value += term;
        store_result_word(result[axis], value);
    }
    copy_result(output, result);
}
} // namespace nfs::graphics::d3dx
