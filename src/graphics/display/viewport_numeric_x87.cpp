#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "math/scalar/retained_transcendentals.hpp"
#include <cstdint>
#include <cstring>
#include <initializer_list>

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Viewport numbers require native x87 intermediates");
namespace nfs::graphics::viewports {
namespace {
float *values(Dword *object) { return reinterpret_cast<float *>(object); }
float &value(Dword *object, unsigned offset) { return values(object)[offset / 4]; }
__declspec(noinline) Dword argument_bits(const float *input) {
    Dword bits;
    std::memcpy(&bits, input, 4);
    return bits;
}
}
namespace numeric_x87 {

void perspective_prefix(Dword *object, Dword degree_bits, Dword aspect_bits,
                        Dword near_bits, Dword far_bits) {
    float degrees, aspect, near_plane, far_plane;
    std::memcpy(&degrees, &degree_bits, sizeof(Dword));
    std::memcpy(&aspect, &aspect_bits, sizeof(Dword));
    std::memcpy(&near_plane, &near_bits, sizeof(Dword));
    std::memcpy(&far_plane, &far_bits, sizeof(Dword));
    auto *matrix = static_cast<volatile Dword *>(object + 52 / 4);
    for (unsigned index : {14u, 13u, 12u, 11u, 9u, 8u, 7u, 6u, 4u, 3u, 2u, 1u})
        matrix[index] = 0;
    for (unsigned index : {15u, 10u, 5u, 0u})
        matrix[index] = 1065353216;
    const float radians = float((static_cast<long double>(3.1415927410125732421875f) *
                                  0.0055555556900799274444580078125f) * degrees);
    math::perspective(values(object) + 52 / 4, radians, 1, near_plane, far_plane);
    const auto horizontal = static_cast<long double>(value(object, 284)) *
                            value(object, 52);
    static_cast<volatile Dword *>(object)[268 / 4] = degree_bits;
    static_cast<volatile Dword *>(object)[272 / 4] = aspect_bits;
    value(object, 52) = float(horizontal);
    const auto vertical = static_cast<long double>(value(object, 288)) *
                          value(object, 72);
    static_cast<volatile Dword *>(object)[276 / 4] = near_bits;
    static_cast<volatile Dword *>(object)[280 / 4] = far_bits;
    static_cast<volatile Dword *>(object)[44 / 4] = 0;
    value(object, 72) = float(vertical * aspect);
    value(object, 84) = float(-static_cast<long double>(value(object, 292)));
    value(object, 88) = float(-static_cast<long double>(value(object, 296)));
}
}


namespace numeric_x87 {
// Keep the native F32 angle stores and the retained bottom angle distinct.
void perspective_tail(Dword *object, float degrees, float aspect) {
    using namespace kernel::numeric::hardware;
    constexpr float pi = 3.1415927410125732421875f;
    constexpr float radians_per_degree = 0.0055555556900799274444580078125f;
    const auto half_angle = ((static_cast<long double>(pi) * radians_per_degree) * degrees) * 0.5f;
    const auto tangent = tangent_retained(&half_angle);
    auto angle_argument = -(tangent * value(object, 340));
    float angles[3];
    angles[0] = static_cast<float>(arctangent_retained(&angle_argument));
    angle_argument = tangent * value(object, 344);
    angles[1] = static_cast<float>(arctangent_retained(&angle_argument));
    angle_argument = (static_cast<long double>(value(object, 348)) / aspect) * tangent;
    angles[2] = static_cast<float>(arctangent_retained(&angle_argument));
    angle_argument = ((-1.0L / aspect) * value(object, 352)) * tangent;
    const auto bottom_angle = arctangent_retained(&angle_argument);
    for (unsigned index = 0; index != 3; ++index) {
        const auto retained_angle = static_cast<long double>(angles[index]);
        value(object, 308 + index * 8) = static_cast<float>(tangent_retained(&retained_angle));
        const auto side_angle = static_cast<long double>(pi) * 0.5f - angles[index];
        value(object, 312 + index * 8) = static_cast<float>(sine_retained(&side_angle));
    }
    value(object, 332) = static_cast<float>(tangent_retained(&bottom_angle));
    const auto side_angle = static_cast<long double>(pi) * 0.5f - bottom_angle;
    value(object, 336) = static_cast<float>(sine_retained(&side_angle));
}
} // namespace numeric_x87

void target_dimensions(const Dword *target, float *width, float *height) {
    *static_cast<volatile float *>(width) =
        static_cast<float>(static_cast<int>(static_cast<const volatile Dword *>(target)[4]));
    *static_cast<volatile float *>(height) =
        static_cast<float>(static_cast<int>(static_cast<const volatile Dword *>(target)[5]));
}


void rectangle(Dword *object, float x, float y, float width, float height, float minimum_depth,
               float maximum_depth) {
    float canvas_width, canvas_height;
    if (object[9])
        backend::get_dimensions(reinterpret_cast<Dword *>(object[9]), &canvas_width, &canvas_height);
    else
        target_dimensions(reinterpret_cast<Dword *>(object[10]), &canvas_width, &canvas_height);
    auto X = [](float value) { return static_cast<long double>(value); };
    auto equal = [](long double a, long double b) {
        return !__builtin_isnan(a) && !__builtin_isnan(b) && a == b;
    };
    float left = x, top = y;
    float right = float((X(x) + width) - 1.0f);
    auto bottom = (X(y) + height) - 1.0f;
    if (x < 0)
        left = 0;
    if (y < 0)
        top = 0;
    if (right < 0)
        right = 0;
    if (bottom < X(0))
        bottom = X(0);
    if (canvas_width < left)
        left = float(X(canvas_width) - 1.0f);
    if (canvas_height < top)
        top = float(X(canvas_height) - 1.0f);
    if (canvas_width < right)
        right = float(X(canvas_width) - 1.0f);
    if (X(canvas_height) < bottom)
        bottom = X(canvas_height) - 1.0f;
    Dword integral_left = Dword(static_cast<std::int64_t>(X(left)));
    object[3] = integral_left;
    Dword integral_top = Dword(static_cast<std::int64_t>(X(top)));
    object[4] = integral_top;
    object[5] = Dword(static_cast<std::int64_t>(X(right))) - integral_left + 1;
    object[6] = Dword(static_cast<std::int64_t>(bottom)) - integral_top + 1;
    value(object, 28) = minimum_depth;
    value(object, 32) = maximum_depth;
    auto retained_span_x = X(right) - left;
    float span_x = float(retained_span_x);
    auto span_y = bottom - top;
    if (!equal(retained_span_x, X(width)) || !equal(span_y, X(height))) {
        if (object[5] * object[6] != 0) {
            auto inverse_x = X(1) / X(span_x);
            float inverse_y = float(X(1) / span_y);
            value(object, 284) = float(inverse_x * width);
            value(object, 288) = float(X(inverse_y) * height);
            auto half_width = X(width) * 0.5f;
            width = float(half_width);
            x = float(half_width + x);
            float offset_x = float(X(x) - (X(span_x) * 0.5f + left));
            auto half_height = X(height) * 0.5f;
            height = float(half_height);
            y = float(half_height + y);
            float offset_y = float(-(X(y) - (span_y * 0.5f + top)));
            auto retained_horizontal = inverse_x * offset_x;
            float horizontal = float(retained_horizontal + retained_horizontal);
            auto retained_vertical = X(offset_y) * inverse_y;
            float vertical = float(retained_vertical + retained_vertical);
            value(object, 300) = value(object, 292) = horizontal;
            value(object, 304) = value(object, 296) = vertical;
            value(object, 340) = float((X(x) - left) / X(width));
            value(object, 344) = float((X(right) - x) / X(width));
            value(object, 348) = float((X(y) - top) / X(height));
            value(object, 352) = float((bottom - y) / X(height));
            return;
        }
        object[3] = object[4] = object[5] = object[6] = 0;
    }
    value(object, 296) = value(object, 292) = 0;
    value(object, 304) = value(object, 300) = 0;
    value(object, 288) = value(object, 284) = 1;
    value(object, 340) = value(object, 344) = 1;
    value(object, 348) = value(object, 352) = 1;
}


void read_rectangle(const Dword *object, float *x, float *y, float *width, float *height,
                    Dword *minimum_depth, Dword *maximum_depth) {
    const auto *input = static_cast<const volatile Dword *>(object);
    *static_cast<volatile float *>(x) = static_cast<float>(input[3]);
    *static_cast<volatile float *>(y) = static_cast<float>(input[4]);
    *static_cast<volatile float *>(width) = static_cast<float>(input[5]);
    *static_cast<volatile float *>(height) = static_cast<float>(input[6]);
    *static_cast<volatile Dword *>(minimum_depth) = input[7];
    *static_cast<volatile Dword *>(maximum_depth) = input[8];
}


void orthographic(Dword *object, float height, float near_plane, float far_plane) {
    const auto retained_height = static_cast<long double>(height);
    auto *matrix = static_cast<volatile Dword *>(object + 52 / 4);
    for (unsigned index : {15u, 10u, 5u, 0u})
        matrix[index] = 1065353216;
    const auto retained_double_height = retained_height + retained_height;
    for (unsigned index : {14u, 13u, 12u, 11u, 9u})
        matrix[index] = 0;
    const float double_height = float(retained_double_height);
    for (unsigned index : {8u, 7u, 6u, 4u, 3u, 2u, 1u})
        matrix[index] = 0;
    math::orthographic(values(object) + 52 / 4, 2, double_height, near_plane, far_plane);
    static_cast<volatile Dword *>(object)[268 / 4] = 0;
    static_cast<volatile Dword *>(object)[272 / 4] = argument_bits(&height);
    static_cast<volatile Dword *>(object)[276 / 4] = argument_bits(&near_plane);
    static_cast<volatile Dword *>(object)[280 / 4] = argument_bits(&far_plane);
    static_cast<volatile Dword *>(object)[44 / 4] = 1;
}


void pixel_orthographic(Dword *object, float near_plane, float far_plane) {
    math::orthographic_bounds(values(object) + 52 / 4, 0, static_cast<float>(object[5]),
                              static_cast<float>(object[6]), 0, near_plane, far_plane);
    static_cast<volatile Dword *>(object)[268 / 4] = 0;
    static_cast<volatile Dword *>(object)[272 / 4] = 1065353216;
    static_cast<volatile Dword *>(object)[276 / 4] = argument_bits(&near_plane);
    static_cast<volatile Dword *>(object)[280 / 4] = argument_bits(&far_plane);
    static_cast<volatile Dword *>(object)[44 / 4] = 1;
}

} // namespace nfs::graphics::viewports
