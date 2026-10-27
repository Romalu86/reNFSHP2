#include "d3dx_packed_matrix_values.hpp"
#include "d3dx_packed_values.hpp"
namespace nfs::graphics::d3dx {
using namespace packed;
namespace {
const float *projection_product(float *temporary, const float *projection, const float *view,
                                const float *world, bool bracket) {
    unsigned selection = (projection ? 1 : 0) | (view ? 2 : 0) | (world ? 4 : 0);
    if (!selection) {
        for (unsigned i = 0; i < 16; ++i)
            temporary[i] = 0;
        temporary[0] = temporary[5] = temporary[10] = temporary[15] = 1;
        return temporary;
    }
    if (selection == 1)
        return projection;
    if (selection == 2)
        return view;
    if (selection == 4)
        return world;
    if (bracket)
        _m_femms();
    switch (selection) {
    case 3:
        multiply_matrix_rows(temporary, view, projection);
        break;
    case 5:
        multiply_matrix_rows(temporary, world, projection);
        break;
    case 6:
        multiply_matrix_rows(temporary, world, view);
        break;
    case 7:
        multiply_matrix_rows(temporary, world, view);
        multiply_matrix_rows(temporary, temporary, projection);
        break;
    }
    if (bracket)
        _m_femms();
    return temporary;
}
__m64 viewport_pair(const Dword *values) {
    __m64 result;
    std::memcpy(&result, values, 8);
    return result;
}
float *unproject_packed(float *out, const float *point, const Dword *viewport,
                        const float *projection, const float *view, const float *world,
                        bool enhanced) {
    float inverse[16];
    if (!enhanced && (projection || view || world))
        _m_femms();
    const float *combined = projection_product(inverse, projection, view, world, false);
    if (projection || view || world) {
        if (enhanced)
            invert_matrix_packed_extended(inverse, nullptr, combined);
        else {
            invert_cofactor_pairs(inverse, nullptr, combined);
            _m_femms();
        }
    }
    if (viewport) {
        _m_femms();
        auto origin = _m_pi2fd(viewport_pair(viewport));
        auto depth = viewport_pair(viewport + 4);
        auto dimensions = _m_pi2fd(viewport_pair(viewport + 2));
        auto estimate = _m_punpckldq(_m_pfrcp(dimensions), _m_pfrcp(repeat_high(dimensions)));
        auto xy = _m_pfsub(load(point), origin);
        auto inverse_size = _m_pfrcpit2(_m_pfrcpit1(dimensions, estimate), estimate);
        inverse_size = _m_pfmul(inverse_size, bits(1073741824, 1073741824));
        xy = _m_pfsub(_m_pfmul(xy, inverse_size), bits(1065353216, 1065353216));
        auto range = _m_pfsub(repeat_high(depth), depth);
        auto inverse_depth = reciprocal(range);
        auto z = _m_pfmul(_m_pfsub(low(point + 2), depth), inverse_depth);
        store(out, _m_pxor(xy, bits(int(2147483648u), 0)));
        store_low(out + 2, z);
        _m_femms();
        point = out;
    }
    transform_coordinate3_packed(out, point, inverse);
    return out;
}
} // namespace

float *__stdcall project_point3_packed(float *out, const float *point, const Dword *viewport,
                                       const float *projection, const float *view,
                                       const float *world) {
    float temporary[16];
    const float *combined = projection_product(temporary, projection, view, world, true);
    transform_coordinate3_packed(out, point, combined);
    if (viewport) {
        _m_femms();
        auto depth = viewport_pair(viewport + 4);
        auto origin = _m_pi2fd(viewport_pair(viewport));
        auto dimensions = _m_pi2fd(viewport_pair(viewport + 2));
        auto xy =
            _m_pfadd(_m_pxor(load(out), bits(int(2147483648u), 0)), bits(1065353216, 1065353216));
        dimensions = _m_pfmul(dimensions, bits(1056964608, 1056964608));
        auto range = _m_pfsub(repeat_high(depth), depth);
        xy = _m_pfmul(xy, dimensions);
        auto z = _m_pfadd(_m_pfmul(low(out + 2), range), depth);
        store(out, _m_pfadd(xy, origin));
        store_low(out + 2, z);
        _m_femms();
    }
    return out;
}

float *__stdcall unproject_point3_packed(float *out, const float *point, const Dword *viewport,
                                         const float *projection, const float *view,
                                         const float *world) {
    return unproject_packed(out, point, viewport, projection, view, world, false);
}

float *__stdcall unproject_point3_packed_extended(float *out, const float *point,
                                                  const Dword *viewport, const float *projection,
                                                  const float *view, const float *world) {
    return unproject_packed(out, point, viewport, projection, view, world, true);
}
} // namespace nfs::graphics::d3dx
