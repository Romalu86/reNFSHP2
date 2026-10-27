#pragma once
#include "d3dx_matrix_dispatch.hpp"
#include <xmmintrin.h>
#include <mm3dnow.h>
namespace nfs::graphics::d3dx::trigonometry {
alignas(16) inline constexpr Dword cosine_constant[4] = {1065353216u, 1065353216u, 1065353216u,
                                                        1065353216u};
alignas(16) inline constexpr Dword cosine_linear[4] = {3204448256u, 3204448256u, 3204448256u,
                                                      3204448256u};
alignas(16) inline constexpr Dword sine_linear[4] = {1065353216u, 1065353216u, 1065353216u,
                                                    1065353216u};
alignas(16) inline constexpr Dword cosine_quadratic[4] = {1026206373u, 1026206373u, 1026206373u,
                                                         1026206373u};
alignas(16) inline constexpr Dword sine_quadratic[4] = {3190467235u, 3190467235u, 3190467235u,
                                                       3190467235u};
alignas(16) inline constexpr Dword cosine_cubic[4] = {3132491290u, 3132491290u, 3132491290u,
                                                     3132491290u};
alignas(16) inline constexpr Dword sine_cubic[4] = {1007190941u, 1007190941u, 1007190941u,
                                                   1007190941u};
alignas(16) inline constexpr Dword quadrant_sign_mask[4] = {2u, 2u, 2u,
                                                           2u};
alignas(16) inline constexpr Dword quadrant_parity_mask[4] = {1u, 1u, 1u,
                                                             1u};
alignas(16) inline constexpr Dword quadrant_mask[4] = {8388607u, 8388607u, 8388607u,
                                                      8388607u};
alignas(16) inline constexpr Dword cosine_quartic[4] = {936179150u, 936179150u, 936179150u,
                                                       936179150u};
alignas(16) inline constexpr Dword sine_quartic[4] = {3108807152u, 3108807152u, 3108807152u,
                                                     3108807152u};
alignas(16) inline constexpr Dword rounding_bias[4] = {1258291200u, 1258291200u, 1258291200u,
                                                      1258291200u};
alignas(16) inline constexpr Dword reduction_tail3[4] = {741630234u, 741630234u, 741630234u,
                                                        741630234u};
alignas(16) inline constexpr Dword reduction_tail2[4] = {866263040u, 866263040u, 866263040u,
                                                        866263040u};
alignas(16) inline constexpr Dword reduction_tail1[4] = {972922880u, 972922880u, 972922880u,
                                                        972922880u};
alignas(16) inline constexpr Dword reduction_leading[4] = {1070137344u, 1070137344u, 1070137344u,
                                                          1070137344u};
alignas(16) inline constexpr Dword two_over_pi[4] = {1059256707u, 1059256707u, 1059256707u,
                                                    1059256707u};
alignas(16) inline constexpr Dword sign_mask[4] = {2147483648u, 2147483648u, 2147483648u,
                                                  2147483648u};
alignas(16) inline constexpr Dword absolute_mask[4] = {2147483647u, 2147483647u, 2147483647u,
                                                      2147483647u};
alignas(16) inline constexpr Dword paired_constants[13][4] = {
    {0u, 1065353216u, 0u, 1065353216u},
    {1065353216u, 3204448256u, 1065353216u, 3204448256u},
    {3190467235u, 1026206373u, 3190467235u, 1026206373u},
    {1007190941u, 3132491290u, 1007190941u, 3132491290u},
    {3108807152u, 936179150u, 3108807152u, 936179150u},
    {2u, 2u, 2u, 2u},
    {1u, 1u, 1u, 1u},
    {8388607u, 8388607u, 8388607u, 8388607u},
    {1258291200u, 1258291200u, 1258291200u, 1258291200u},
    {1070137344u, 972922880u, 866263040u, 741630234u},
    {1059256707u, 1059256707u, 1059256707u, 1059256707u},
    {2147483648u, 2147483648u, 2147483648u, 2147483648u},
    {2147483647u, 2147483647u, 2147483647u, 2147483647u},
};
inline __m128 vector(const Dword *words) {
    return _mm_load_ps(reinterpret_cast<const float *>(words));
}
void approximate_sine_cosine_sse(const float *, float *, float *);
void approximate_sine_cosine_sse2(const float *, float *, float *);
__m128 __stdcall approximate_sine_cosine_pair(__m128);
inline constexpr Dword packed_unit[2] = {1065353216u, 1065353216u};
inline constexpr Dword packed_reduction[2] = {3209232384u, 3112020514u};
inline constexpr Dword packed_polynomial_a[2] = {3163703589u, 3166899249u};
inline constexpr Dword packed_half_pi[2] = {1061752795u, 1061752795u};
inline constexpr Dword packed_polynomial_b[2] = {3171453065u, 3175926989u};
inline constexpr Dword packed_polynomial_c[2] = {3182078635u, 3190467243u};
inline constexpr Dword packed_polynomial_d[2] = {3204448256u, 3204448256u};
inline constexpr Dword packed_absolute_mask = 2147483647u;
inline constexpr Dword packed_invalid_result = 1065353216u;
inline constexpr Dword packed_two_over_pi = 1067645315u;
inline constexpr Dword packed_argument_limit = 1187581915u;
__m64 approximate_sine_cosine_packed(__m64);
__m64 approximate_sine_packed(__m64);
__m64 approximate_acos_packed(__m64);
__m64 approximate_atan2_packed(__m64, __m64);
} // namespace nfs::graphics::d3dx::trigonometry
