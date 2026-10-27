#include "d3dx_trigonometric_constants.hpp"
namespace nfs::graphics::d3dx::trigonometry {
namespace {
void sine_cosine_vectors(const float *angles, float *sines, float *cosines) {
    auto input = _mm_load_ps(angles);
    auto absolute = _mm_and_ps(input, vector(absolute_mask));
    auto signs = _mm_and_ps(input, vector(sign_mask));
    auto rounded = _mm_add_ps(_mm_mul_ps(absolute, vector(two_over_pi)), vector(rounding_bias));
    auto quadrants = _mm_sub_ps(rounded, vector(rounding_bias));
    auto reduced = _mm_sub_ps(absolute, _mm_mul_ps(vector(reduction_leading), quadrants));
    reduced = _mm_sub_ps(reduced, _mm_mul_ps(vector(reduction_tail1), quadrants));
    reduced = _mm_sub_ps(reduced, _mm_mul_ps(vector(reduction_tail2), quadrants));
    reduced = _mm_sub_ps(reduced, _mm_mul_ps(vector(reduction_tail3), quadrants));
    auto square = _mm_mul_ps(reduced, reduced);
    alignas(16) Dword quadrant_words[4], odd_words[4], sine_sign[4], cosine_sign[4];
    _mm_store_ps(reinterpret_cast<float *>(quadrant_words),
                 _mm_and_ps(rounded, vector(quadrant_mask)));
    for (unsigned i = 0; i < 4; ++i) {
        Dword odd = quadrant_words[i] & quadrant_parity_mask[i];
        sine_sign[i] = ((quadrant_words[i] - odd) & quadrant_sign_mask[i]) << 30;
        cosine_sign[i] = ((quadrant_words[i] + odd) & quadrant_sign_mask[i]) << 30;
        odd_words[i] = odd << 30;
    }
    auto selector = _mm_cmpneq_ps(vector(odd_words), _mm_setzero_ps());
    auto sine = _mm_add_ps(_mm_mul_ps(vector(sine_quartic), square), vector(sine_cubic));
    sine = _mm_add_ps(_mm_mul_ps(sine, square), vector(sine_quadratic));
    sine = _mm_add_ps(_mm_mul_ps(sine, square), vector(sine_linear));
    sine = _mm_mul_ps(sine, reduced);
    auto cosine = _mm_add_ps(_mm_mul_ps(vector(cosine_quartic), square), vector(cosine_cubic));
    cosine = _mm_add_ps(_mm_mul_ps(cosine, square), vector(cosine_quadratic));
    cosine = _mm_add_ps(_mm_mul_ps(cosine, square), vector(cosine_linear));
    cosine = _mm_add_ps(_mm_mul_ps(cosine, square), vector(cosine_constant));
    auto sine_result = _mm_or_ps(_mm_andnot_ps(selector, sine), _mm_and_ps(selector, cosine));
    auto cosine_result = _mm_or_ps(_mm_and_ps(selector, sine), _mm_andnot_ps(selector, cosine));
    _mm_store_ps(sines, _mm_xor_ps(sine_result, _mm_xor_ps(vector(sine_sign), signs)));
    _mm_store_ps(cosines, _mm_xor_ps(cosine_result, vector(cosine_sign)));
}
} // namespace

void approximate_sine_cosine_sse(const float *angles, float *sines, float *cosines) {
    sine_cosine_vectors(angles, sines, cosines);
}
// Coefficient vectors equal those of .
void approximate_sine_cosine_sse2(const float *angles, float *sines, float *cosines) {
    sine_cosine_vectors(angles, sines, cosines);
}
// XMM input/result and RET16 have a C++ value adapter.
__m128 __stdcall approximate_sine_cosine_pair(__m128 input) {
    auto absolute = _mm_and_ps(input, vector(paired_constants[12]));
    auto signs = _mm_and_ps(input, vector(paired_constants[11]));
    auto rounded =
        _mm_add_ps(_mm_mul_ps(absolute, vector(paired_constants[10])), vector(paired_constants[8]));
    auto quadrants = _mm_sub_ps(rounded, vector(paired_constants[8]));
    auto reduction = _mm_mul_ps(vector(paired_constants[9]), quadrants);
    auto reduced = _mm_sub_ps(absolute, reduction);
    for (unsigned i = 0; i < 3; ++i) {
        reduction = _mm_shuffle_ps(reduction, reduction, 57);
        reduced = _mm_sub_ps(reduced, reduction);
    }
    auto square = _mm_mul_ps(reduced, reduced);
    alignas(16) Dword quadrant_words[4], odd_words[4], sine_sign[4], cosine_sign[4];
    _mm_store_ps(reinterpret_cast<float *>(quadrant_words),
                 _mm_and_ps(rounded, vector(paired_constants[7])));
    for (unsigned i = 0; i < 4; ++i) {
        Dword odd = quadrant_words[i] & paired_constants[6][i];
        sine_sign[i] = ((quadrant_words[i] - odd) & paired_constants[5][i]) << 30;
        cosine_sign[i] = ((quadrant_words[i] + odd) & paired_constants[5][i]) << 30;
        odd_words[i] = odd << 30;
    }
    auto selector = _mm_cmpneq_ps(vector(odd_words), _mm_setzero_ps());
    auto polynomial =
        _mm_add_ps(_mm_mul_ps(vector(paired_constants[4]), square), vector(paired_constants[3]));
    polynomial = _mm_add_ps(_mm_mul_ps(polynomial, square), vector(paired_constants[2]));
    polynomial = _mm_add_ps(_mm_mul_ps(polynomial, square), vector(paired_constants[1]));
    polynomial = _mm_add_ps(_mm_mul_ps(polynomial, _mm_unpacklo_ps(reduced, square)),
                            vector(paired_constants[0]));
    auto selected = _mm_and_ps(selector, polynomial);
    auto other = _mm_andnot_ps(selector, polynomial);
    other = _mm_shuffle_ps(other, other, 177);
    auto sign = _mm_unpacklo_ps(vector(cosine_sign), _mm_xor_ps(vector(sine_sign), signs));
    return _mm_xor_ps(_mm_or_ps(selected, other), sign);
}
} // namespace nfs::graphics::d3dx::trigonometry
