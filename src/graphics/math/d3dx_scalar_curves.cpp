#include "d3dx_dispatch_kernels.hpp"

static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Scalar curves require the retained x87 compiler.");

namespace nfs::graphics::d3dx {
namespace {
template <unsigned Dimensions, bool Halve, typename Coefficient>
void evaluate_polynomial(float *output, const float *first, const float *second,
                         const float *third, const float *fourth,
                         const volatile Coefficient *coefficients,
                         long double fourth_coefficient) {
    volatile float *destination = output;
    const volatile float *first_point = first;
    const volatile float *second_point = second;
    const volatile float *third_point = third;
    const volatile float *fourth_point = fourth;

    long double value = static_cast<long double>(coefficients[0]) * first_point[0];
    long double term = static_cast<long double>(coefficients[1]) * second_point[0];
    value += term;
    term = static_cast<long double>(coefficients[2]) * third_point[0];
    value += term;
    term = fourth_coefficient * fourth_point[0];
    value += term;
    if constexpr (Halve)
        value *= .5f;
    destination[0] = static_cast<float>(value);

    // Later components start with the retained fourth coefficient. Each output
    // store precedes reads of the next component, including overlapping inputs.
    for (unsigned axis = 1; axis < Dimensions; ++axis) {
        value = fourth_coefficient * fourth_point[axis];
        term = static_cast<long double>(coefficients[2]) * third_point[axis];
        value += term;
        term = static_cast<long double>(coefficients[1]) * second_point[axis];
        value += term;
        term = static_cast<long double>(coefficients[0]) * first_point[axis];
        value += term;
        if constexpr (Halve)
            value *= .5f;
        destination[axis] = static_cast<float>(value);
    }
}

template <unsigned Dimensions, bool Catmull>
void polynomial_curve(float *output, const float *first, const float *second, const float *third,
                      const float *fourth, float time) {
    const long double square = static_cast<long double>(time) * time;
    const long double cube = static_cast<long double>(time) * square;
    volatile float coefficients[3];
    if constexpr (Catmull) {
        coefficients[0] = static_cast<float>(square * 2.0f - cube - time);
        const long double triple_cube = cube * 3.0f;
        const long double five_squares = square * 5.0f;
        coefficients[1] = static_cast<float>(triple_cube - five_squares + 2.0f);
        coefficients[2] = static_cast<float>(square * 4.0f - triple_cube + time);
    } else {
        const long double double_cube = cube + cube;
        const long double triple_square = square * 3.0f;
        coefficients[0] = static_cast<float>(double_cube - triple_square + 1.0f);
        coefficients[1] = static_cast<float>(cube - (square + square) + time);
        coefficients[2] = static_cast<float>(triple_square - double_cube);
    }
    const long double fourth_coefficient = cube - square;
    evaluate_polynomial<Dimensions, Catmull>(output, first, second, third, fourth, coefficients,
                                            fourth_coefficient);
}

template <unsigned Dimensions>
void barycentric_curve(float *output, const float *first, const float *second, const float *third,
                       float first_weight, float second_weight) {
    volatile float *destination = output;
    const volatile float *first_point = first;
    const volatile float *second_point = second;
    const volatile float *third_point = third;
    for (unsigned axis = 0; axis < Dimensions; ++axis) {
        long double first_term = static_cast<long double>(second_point[axis]);
        first_term -= first_point[axis];
        first_term *= first_weight;
        long double second_term = static_cast<long double>(third_point[axis]);
        second_term -= first_point[axis];
        second_term *= second_weight;
        first_term += second_term;
        first_term += first_point[axis];
        destination[axis] = static_cast<float>(first_term);
    }
}
} // namespace

void __stdcall hermite_vector2_scalar(float *output, const float *first, const float *second,
                                      const float *third, const float *fourth, float time) {
    polynomial_curve<2, false>(output, first, second, third, fourth, time);
}

void __stdcall catmull_vector2_scalar(float *output, const float *first, const float *second,
                                      const float *third, const float *fourth, float time) {
    polynomial_curve<2, true>(output, first, second, third, fourth, time);
}

void __stdcall barycentric_vector2_scalar(float *output, const float *first, const float *second,
                                          const float *third, float first_weight,
                                          float second_weight) {
    barycentric_curve<2>(output, first, second, third, first_weight, second_weight);
}

void __stdcall hermite_vector3_scalar(float *output, const float *first, const float *second,
                                      const float *third, const float *fourth, float time) {
    polynomial_curve<3, false>(output, first, second, third, fourth, time);
}

void __stdcall catmull_vector3_scalar(float *output, const float *first, const float *second,
                                      const float *third, const float *fourth, float time) {
    polynomial_curve<3, true>(output, first, second, third, fourth, time);
}

void __stdcall barycentric_vector3_scalar(float *output, const float *first, const float *second,
                                          const float *third, float first_weight,
                                          float second_weight) {
    barycentric_curve<3>(output, first, second, third, first_weight, second_weight);
}

void __stdcall hermite_vector4_scalar(float *output, const float *first, const float *second,
                                      const float *third, const float *fourth, float time) {
    const long double square = static_cast<long double>(time) * time;
    const long double cube = static_cast<long double>(time) * square;

    // This variant stores the powers/products, then retains the coefficients.
    // Hermite2/3 instead store their first three completed coefficients.
    volatile float stored_cube = static_cast<float>(cube);
    volatile float stored_double_cube = static_cast<float>(cube + cube);
    volatile float stored_triple_square = static_cast<float>(3.0L * square);
    long double coefficients[3];
    coefficients[0] = static_cast<long double>(stored_double_cube) - stored_triple_square + 1.0f;
    coefficients[1] = static_cast<long double>(stored_cube) - (square + square) + time;
    coefficients[2] = static_cast<long double>(stored_triple_square) - stored_double_cube;
    const long double fourth_coefficient = static_cast<long double>(stored_cube) - square;
    evaluate_polynomial<4, false>(output, first, second, third, fourth, coefficients,
                                  fourth_coefficient);
}

void __stdcall catmull_vector4_scalar(float *output, const float *first, const float *second,
                                      const float *third, const float *fourth, float time) {
    polynomial_curve<4, true>(output, first, second, third, fourth, time);
}

void __stdcall barycentric_vector4_scalar(float *output, const float *first, const float *second,
                                          const float *third, float first_weight,
                                          float second_weight) {
    barycentric_curve<4>(output, first, second, third, first_weight, second_weight);
}
} // namespace nfs::graphics::d3dx
