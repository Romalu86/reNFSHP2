#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
void identity_values(float *out) {
    auto zero = _m_from_int(0), one_low = _m_from_int(1065353216), one_high = bits(1065353216, 0);
    store(out, one_low);
    store(out + 2, zero);
    store(out + 4, one_high);
    store(out + 6, zero);
    store(out + 8, zero);
    store(out + 10, one_low);
    store(out + 12, zero);
    store(out + 14, one_high);
}
void scale_values(float *out, __m64 xy, __m64 z) {
    auto zero = _m_from_int(0);
    store(out, _m_punpckldq(xy, zero));
    store(out + 2, zero);
    store(out + 4, _m_punpckhdq(zero, xy));
    store(out + 6, zero);
    store(out + 8, zero);
    store(out + 10, z);
    store(out + 12, zero);
    store(out + 14, bits(1065353216, 0));
}
void translate_values(float *out, const float *translation, bool subtract, bool input_first) {
    auto xy = load(out + 12), z = low(out + 14);
    auto input_xy = load(translation), input_z = low(translation + 2);
    if (subtract) {
        xy = _m_pfsub(xy, input_xy);
        z = _m_pfsub(z, input_z);
    } else if (input_first) {
        xy = _m_pfadd(input_xy, xy);
        z = _m_pfadd(input_z, z);
    } else {
        xy = _m_pfadd(xy, input_xy);
        z = _m_pfadd(z, input_z);
    }
    store(out + 12, xy);
    store_low(out + 14, z);
}
void transpose_values(float *out, const float *matrix) {
    auto a = load(matrix), b = load(matrix + 4), c = load(matrix + 8), d = load(matrix + 12);
    auto e = load(matrix + 2), f = load(matrix + 6), g = load(matrix + 10), h = load(matrix + 14);
    store(out, _m_punpckldq(a, b));
    store(out + 4, _m_punpckhdq(a, b));
    store(out + 10, _m_punpckldq(g, h));
    store(out + 14, _m_punpckhdq(g, h));
    store(out + 8, _m_punpckldq(e, f));
    store(out + 12, _m_punpckhdq(e, f));
    store(out + 6, _m_punpckhdq(c, d));
    store(out + 2, _m_punpckldq(c, d));
}
float *compose_values(float *out, const float *scale_center, const float *scale_rotation,
                      const float *scale, const float *rotation_center, const float *rotation,
                      const float *translation, bool enhanced) {
    float scaling[16], rotation_matrix[16], transpose[16];
    auto make_rotation = [&](float *matrix, const float *quaternion) {
        if (enhanced)
            quaternion_matrix_extended_values(matrix, quaternion);
        else
            quaternion_matrix_values(matrix, quaternion);
    };
    if (!scale)
        identity_values(out);
    else if (!scale_rotation)
        scale_values(out, load(scale), low(scale + 2));
    else {
        scale_values(scaling, load(scale), low(scale + 2));
        make_rotation(rotation_matrix, scale_rotation);
        if (scale_center) {
            transpose_values(transpose, rotation_matrix);
            // Read the center before initializing the output.
            auto center_xy = load(scale_center), center_z = low(scale_center + 2);
            identity_values(out);
            store(out + 12, _m_pfsub(_m_from_int(0), center_xy));
            store(out + 14, _m_pfsub(bits(1065353216, 0), center_z));
            multiply_matrix_rows(out, out, transpose);
            multiply_matrix_rows(out, out, scaling);
            multiply_matrix_rows(out, out, rotation_matrix);
            translate_values(out, scale_center, false, true);
        } else {
            transpose_values(out, rotation_matrix);
            multiply_matrix_rows(out, out, scaling);
            multiply_matrix_rows(out, out, rotation_matrix);
        }
    }
    if (rotation) {
        make_rotation(rotation_matrix, rotation);
        if (rotation_center)
            translate_values(out, rotation_center, true, false);
        multiply_matrix_rows(out, out, rotation_matrix);
        if (rotation_center)
            translate_values(out, rotation_center, false, false);
    }
    if (translation)
        translate_values(out, translation, false, false);
    _m_femms();
    return out;
}
} // namespace

float *__stdcall affine_matrix_packed(float *out, float scale, const float *center,
                                      const float *rotation, const float *translation) {
    auto scalar = low(&scale);
    scale_values(out, repeat_low(scalar), scalar);
    if (rotation) {
        float matrix[16];
        quaternion_matrix_values(matrix, rotation);
        if (center)
            translate_values(out, center, true, false);
        multiply_matrix_rows(out, out, matrix);
        if (center)
            translate_values(out, center, false, false);
    }
    if (translation)
        translate_values(out, translation, false, true);
    _m_femms();
    return out;
}

float *__stdcall transformation_matrix_packed(float *out, const float *scale_center,
                                              const float *scale_rotation, const float *scale,
                                              const float *rotation_center, const float *rotation,
                                              const float *translation) {
    return compose_values(out, scale_center, scale_rotation, scale, rotation_center, rotation,
                          translation, false);
}

float *__stdcall transformation_matrix_packed_extended(
    float *out, const float *scale_center, const float *scale_rotation, const float *scale,
    const float *rotation_center, const float *rotation, const float *translation) {
    return compose_values(out, scale_center, scale_rotation, scale, rotation_center, rotation,
                          translation, true);
}
} // namespace nfs::graphics::d3dx
