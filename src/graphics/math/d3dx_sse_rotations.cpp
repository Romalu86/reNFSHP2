#include "d3dx_dispatch_kernels.hpp"
#include "d3dx_trigonometric_constants.hpp"
namespace nfs::graphics::d3dx {
namespace {
void yaw_pitch_roll_vectors(float *out, float yaw, float pitch, float roll, bool second_backend) {
    alignas(16) float angles[4] = {roll, pitch, yaw, 0}, sines[4], cosines[4];
    auto half_angles = _mm_mul_ps(_mm_load_ps(angles), _mm_set1_ps(.5f));
    _mm_store_ps(angles, half_angles);
    if (second_backend)
        trigonometry::approximate_sine_cosine_sse2(angles, sines, cosines);
    else
        trigonometry::approximate_sine_cosine_sse(angles, sines, cosines);
    auto sine = _mm_load_ps(sines), cosine = _mm_load_ps(cosines);
    auto roll_pair = _mm_shuffle_ps(sine, cosine, 0);
    roll_pair = _mm_shuffle_ps(roll_pair, roll_pair, 138);
    auto pitch_pair = _mm_shuffle_ps(sine, cosine, 153);
    auto roll_other = _mm_shuffle_ps(roll_pair, roll_pair, 138);
    auto pitch_yaw = _mm_shuffle_ps(pitch_pair, pitch_pair, 247);
    auto yaw_pitch = _mm_shuffle_ps(pitch_pair, pitch_pair, 168);
    auto first = _mm_mul_ps(roll_pair, pitch_yaw);
    pitch_yaw = _mm_shuffle_ps(pitch_yaw, pitch_yaw, 81);
    auto second = _mm_mul_ps(roll_other, pitch_yaw);
    first = _mm_mul_ps(first, yaw_pitch);
    yaw_pitch = _mm_shuffle_ps(yaw_pitch, yaw_pitch, 1);
    second = _mm_mul_ps(second, yaw_pitch);
    alignas(16) const Dword signs[4] = {0, 2147483648u, 2147483648u, 0};
    second = _mm_xor_ps(second, trigonometry::vector(signs));
    _mm_storeu_ps(out, _mm_add_ps(first, second));
}
} // namespace

void __stdcall quaternion_yaw_pitch_roll_sse(float *out, float yaw, float pitch, float roll) {
    yaw_pitch_roll_vectors(out, yaw, pitch, roll, false);
}

void __stdcall quaternion_yaw_pitch_roll_sse2(float *out, float yaw, float pitch, float roll) {
    yaw_pitch_roll_vectors(out, yaw, pitch, roll, true);
}

void __stdcall rotation_x_matrix_sse2(float *out, float angle) {
    auto pair = trigonometry::approximate_sine_cosine_pair(_mm_set1_ps(angle));
    auto zero = _mm_set_ps(1, 0, 0, 0);
    _mm_storeu_ps(out, _mm_set_ps(0, 0, 0, 1));
    _mm_store_ss(out + 4, zero);
    _mm_storel_pi(reinterpret_cast<__m64 *>(out + 5), pair);
    auto reverse = _mm_shuffle_ps(pair, pair, 17);
    _mm_storel_pi(reinterpret_cast<__m64 *>(out + 7), zero);
    reverse = _mm_xor_ps(reverse, _mm_set_ss(-0.0f));
    _mm_store_ss(out + 11, zero);
    _mm_storel_pi(reinterpret_cast<__m64 *>(out + 9), reverse);
    _mm_storeu_ps(out + 12, zero);
}

void __stdcall rotation_y_matrix_sse2(float *out, float angle) {
    auto pair = trigonometry::approximate_sine_cosine_pair(_mm_set1_ps(angle));
    pair = _mm_shuffle_ps(pair, pair, 80);
    alignas(16) const Dword mask[4] = {4294967295u, 0, 4294967295u, 0};
    pair = _mm_and_ps(pair, trigonometry::vector(mask));
    auto first = _mm_xor_ps(_mm_set_ps(0, -0.0f, 0, 0), pair);
    _mm_storeu_ps(out, first);
    _mm_storeu_ps(out + 4, _mm_set_ps(0, 0, 1, 0));
    pair = _mm_shuffle_ps(pair, pair, 70);
    _mm_storeu_ps(out + 8, pair);
    _mm_storeu_ps(out + 12, _mm_set_ps(1, 0, 0, 0));
}
} // namespace nfs::graphics::d3dx
