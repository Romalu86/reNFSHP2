#include "d3dx_dispatch_kernels.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::graphics::d3dx {
using kernel::numeric::Extended;

void __stdcall interpolate_quaternion_scalar(float *out, const float *first, const float *second,
                                             float time) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto first_weight = X(1) - time;
    auto dot = X(first[0]) * second[0] + X(second[2]) * first[2] + X(second[1]) * first[1] +
               X(second[3]) * first[3];
    // TEST AH,1 includes the unordered case in the negative hemisphere.
    float sign = dot < X(0) || std::isnan(double(dot)) ? -1.0f : 1.0f;
    auto cosine = X(sign) * dot;
    float stored_cosine = float(cosine);
    auto second_weight = X(time);
    if (X(0.00000011920928955078125f) < X(1) - cosine) {
        auto square = X(1) - cosine * cosine;
        auto sine = square.square_root();
        float angle = float(std::atan2(double(sine), double(stored_cosine)));
        float inverse_sine = float(X(1) / sine);
        first_weight =
            Extended::from_double(std::sin(double(X(angle) * first_weight)), format) * inverse_sine;
        second_weight =
            Extended::from_double(std::sin(double(X(angle) * time)), format) * inverse_sine;
    }
    second_weight = second_weight * sign;
    out[0] = float(first_weight * first[0] + second_weight * second[0]);
    for (unsigned component = 1; component < 4; ++component)
        out[component] = float(second_weight * second[component] + first_weight * first[component]);
}

float *__stdcall barycentric_quaternion_scalar(float *out, const float *origin, const float *first,
                                               const float *second, float u, float v) {
    auto format = kernel::numeric::current_format();
    float sum = float(Extended(u, format) + v);
    if (approximately_equal(sum, 0)) {
        if (out != origin)
            for (unsigned i = 0; i < 4; ++i)
                out[i] = origin[i];
    } else {
        float a[4], b[4];
        interpolate_quaternion_bootstrap(a, origin, first, sum);
        interpolate_quaternion_bootstrap(b, origin, second, sum);
        float ratio = float(Extended(v, format) / Extended(sum, format));
        interpolate_quaternion_bootstrap(out, a, b, ratio);
    }
    return out;
}

float *__stdcall squad_quaternion_scalar(float *out, const float *first, const float *control_first,
                                         const float *control_second, const float *second,
                                         float time) {
    float endpoints[4], controls[4];
    interpolate_quaternion_bootstrap(endpoints, first, second, time);
    interpolate_quaternion_bootstrap(controls, control_first, control_second, time);
    auto format = kernel::numeric::current_format();
    auto weight = (Extended(1, format) - time) * time;
    float doubled_weight = float(weight + weight);
    interpolate_quaternion_bootstrap(out, endpoints, controls, doubled_weight);
    return out;
}

void __stdcall setup_squad_scalar(float *first_control, float *second_control, float *second_out,
                                  const float *previous, const float *first, const float *second,
                                  const float *next) {
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    float endpoints[4][4];
    const float *inputs[] = {previous, first, second, next};
    for (unsigned q = 0; q < 4; ++q)
        for (unsigned i = 0; i < 4; ++i)
            endpoints[q][i] = inputs[q][i];
    auto align = [&](float *candidate, const float *anchor) {
        auto sum_x = X(candidate[0]) + anchor[0], sum_y = X(candidate[1]) + anchor[1];
        auto sum_z = X(candidate[2]) + anchor[2], sum_w = X(candidate[3]) + anchor[3];
        auto difference_x = X(anchor[0]) - candidate[0];
        float difference_y = float(X(anchor[1]) - candidate[1]);
        float difference_z = float(X(anchor[2]) - candidate[2]);
        float difference_w = float(X(anchor[3]) - candidate[3]);
        auto sum_square = sum_w * sum_w + sum_z * sum_z + sum_y * sum_y + sum_x * sum_x;
        auto difference_square = X(difference_w) * difference_w + X(difference_z) * difference_z +
                                 X(difference_y) * difference_y + difference_x * difference_x;
        if (sum_square < difference_square)
            for (unsigned i = 0; i < 4; ++i)
                candidate[i] = -candidate[i];
    };
    align(endpoints[0], endpoints[1]);
    align(endpoints[2], endpoints[1]);
    align(endpoints[3], endpoints[2]);
    float inverse[4], product[4], left_log[4], right_log[4], exponent[4], tangent[4];
    for (unsigned segment = 0; segment < 2; ++segment) {
        inverse_quaternion_bootstrap(inverse, endpoints[segment + 1]);
        multiply_quaternion_bootstrap(product, inverse, endpoints[segment]);
        logarithm_quaternion_scalar(left_log, product);
        multiply_quaternion_bootstrap(product, inverse, endpoints[segment + 2]);
        logarithm_quaternion_scalar(right_log, product);
        for (unsigned i = 0; i < 4; ++i) {
            auto sum = X(left_log[i]) + right_log[i];
            if (i == 1 || i == 2)
                sum = X(float(sum));
            tangent[i] = float(sum * -.25f);
        }
        exponential_quaternion_scalar(exponent, tangent);
        multiply_quaternion_bootstrap(product, endpoints[segment + 1], exponent);
        float *destination = segment ? second_control : first_control;
        for (unsigned i = 0; i < 4; ++i)
            destination[i] = product[i];
    }
    for (unsigned i = 0; i < 4; ++i)
        second_out[i] = endpoints[2][i];
}
} // namespace nfs::graphics::d3dx
