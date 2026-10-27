#include "nfs/graphics/textures/pixel_compression.hpp"
#include <cfenv>
#include <cfloat>
#include <utility>

namespace nfs::graphics::surfaces::pixels {
namespace {

class TruncationScope {
  public:
    TruncationScope() : previous_(std::fegetround()) {
        std::fesetround(FE_TOWARDZERO);
    }
    ~TruncationScope() {
        std::fesetround(previous_);
    }

  private:
    int previous_;
};


constexpr float alpha_upper8[8] = {0,
                                   0.14285714924335479736328125f,
                                   0.2857142984867095947265625f,
                                   0.4285714328289031982421875f,
                                   0.571428596973419189453125f,
                                   0.71428573131561279296875f,
                                   0.857142865657806396484375f,
                                   1.14285719394683837890625f};
constexpr float alpha_lower8[8] = {1,
                                   0.857142865657806396484375f,
                                   0.71428573131561279296875f,
                                   0.571428596973419189453125f,
                                   0.4285714328289031982421875f,
                                   0.2857142984867095947265625f,
                                   0.14285714924335479736328125f,
                                   0};
constexpr float alpha_upper6[6] = {
    0, 0.20000000298023223876953125f, 0.4000000059604644775390625f, 0.60000002384185791015625f, 0.800000011920928955078125f, 1};
constexpr float alpha_lower6[6] = {
    1, 0.800000011920928955078125f, 0.60000002384185791015625f, 0.4000000059604644775390625f, 0.20000000298023223876953125f, 0};
constexpr float colour_upper4[4] = {0, 0.3333333432674407958984375f, 0.666666686534881591796875f, 1};
constexpr float colour_lower4[4] = {1, 0.666666686534881591796875f, 0.3333333432674407958984375f, 0};
constexpr float colour_upper3[3] = {0, 0.5f, 1};
constexpr float colour_lower3[3] = {1, 0.5f, 0};
constexpr float colour_weights[3] = {0.297036647796630859375f, 1, 0.1007827818393707275390625f};

float clamp_unit(float value) {
    if (!(value >= 0))
        return 0;
    return value > 1 ? 1 : value;
}

} // namespace

// Clamp before the saved round-toward-zero RGB565 conversion.
Dword pack_565(const float *colour) {
    float components[3] = {clamp_unit(colour[0]), clamp_unit(colour[1]), clamp_unit(colour[2])};
    TruncationScope rounding;
    float red = static_cast<double>(components[0]) * 31 + 0.5;
    float green = static_cast<double>(components[1]) * 63 + 0.5;
    float blue = static_cast<double>(components[2]) * 31 + 0.5;
    return ((Dword(static_cast<int>(red)) << 6 | Dword(static_cast<int>(green))) << 5) |
           Dword(static_cast<int>(blue));
}

// Up to eight least-squares endpoint updates.
void fit_alpha_endpoints(float *lower_out, float *upper_out, const float *samples, Dword levels) {
    const float *lower_weight = levels == 6 ? alpha_lower6 : alpha_lower8;
    const float *upper_weight = levels == 6 ? alpha_upper6 : alpha_upper8;
    float lower = 1;
    float upper = 0;
    for (unsigned pixel = 0; pixel < 16; ++pixel) {
        float value = samples[pixel];
        if (value < lower && (levels == 8 || value > 0))
            lower = value;
        if (value > upper && (levels == 8 || value < 1))
            upper = value;
    }

    TruncationScope rounding;
    float last = static_cast<float>(levels - 1);
    for (unsigned iteration = 0; iteration < 8; ++iteration) {
        double difference = static_cast<double>(upper) - lower;
        if (difference < 0.00390625f)
            break;
        float scale = static_cast<double>(last) / difference;
        float palette[8];
        for (Dword entry = 0; entry < levels; ++entry) {
            palette[entry] = static_cast<double>(lower) * lower_weight[entry] +
                             static_cast<double>(upper) * upper_weight[entry];
        }
        if (levels == 6) {
            palette[6] = 0;
            palette[7] = 1;
        }

        float lower_gradient = 0, upper_gradient = 0;
        float lower_curvature = 0, upper_curvature = 0;
        for (unsigned pixel = 0; pixel < 16; ++pixel) {
            float value = samples[pixel];
            double position = (static_cast<double>(value) - lower) * scale;
            Dword selected;
            if (position > 0) {
                if (position < last) {
                    float biased = static_cast<double>(position) + 0.5;
                    selected = static_cast<Dword>(static_cast<int>(biased));
                } else {
                    selected = levels - 1;
                    if (levels == 6 && (static_cast<double>(upper) + 1) * 0.5 < value)
                        continue;
                }
            } else {
                if (levels == 6 && !(static_cast<double>(lower) * 0.5 < value))
                    continue;
                selected = 0;
            }
            if (selected < levels) {
                double residual = static_cast<double>(value) - palette[selected];
                lower_gradient = residual * lower_weight[selected] + lower_gradient;
                lower_curvature =
                    static_cast<double>(lower_weight[selected]) * lower_weight[selected] +
                    lower_curvature;
                upper_gradient = residual * upper_weight[selected] + upper_gradient;
                upper_curvature =
                    static_cast<double>(upper_weight[selected]) * upper_weight[selected] +
                    upper_curvature;
            }
        }
        if (lower_curvature > 0)
            lower =
                static_cast<double>(lower) - static_cast<double>(lower_gradient) / lower_curvature;
        if (upper_curvature > 0)
            upper =
                static_cast<double>(upper) - static_cast<double>(upper_gradient) / upper_curvature;
        if (upper < lower)
            std::swap(lower, upper);
        if (static_cast<double>(lower_gradient) * lower_gradient < 0.015625 &&
            static_cast<double>(upper_gradient) * upper_gradient < 0.015625)
            break;
    }
    *lower_out = clamp_unit(lower);
    *upper_out = clamp_unit(upper);
}

// Select the initial box diagonal, then refine both endpoints.
void fit_colour_endpoints(float *lower_out, float *upper_out, const Colour *samples, Dword levels) {
    const float *lower_weight = levels == 3 ? colour_lower3 : colour_lower4;
    const float *upper_weight = levels == 3 ? colour_upper3 : colour_upper4;
    float lower[3] = {colour_weights[0], colour_weights[1], colour_weights[2]};
    float upper[3] = {0, 0, 0};
    for (unsigned pixel = 0; pixel < 16; ++pixel) {
        const auto *value = reinterpret_cast<const float *>(samples + pixel);
        for (unsigned channel = 0; channel < 3; ++channel) {
            if (value[channel] < lower[channel])
                lower[channel] = value[channel];
            if (value[channel] > upper[channel])
                upper[channel] = value[channel];
        }
    }
    double diagonal[3];
    for (unsigned channel = 0; channel < 3; ++channel)
        diagonal[channel] = static_cast<double>(upper[channel]) - lower[channel];
    diagonal[0] = static_cast<float>(diagonal[0]);
    diagonal[1] = static_cast<float>(diagonal[1]);
    double squared_length =
        diagonal[2] * diagonal[2] + diagonal[1] * diagonal[1] + diagonal[0] * diagonal[0];
    float length = squared_length;
    if (length >= FLT_MIN) {
        double inverse = 1.0 / squared_length;
        float normalized[3];
        for (unsigned channel = 0; channel < 3; ++channel)
            normalized[channel] = diagonal[channel] * inverse;
        float centre[3];
        for (unsigned channel = 0; channel < 3; ++channel)
            centre[channel] = (static_cast<double>(upper[channel]) + lower[channel]) * 0.5;
        float variance[4] = {};
        for (unsigned pixel = 0; pixel < 16; ++pixel) {
            const auto *value = reinterpret_cast<const float *>(samples + pixel);
            float red = (static_cast<double>(value[0]) - centre[0]) * normalized[0];
            float green = (static_cast<double>(value[1]) - centre[1]) * normalized[1];
            double blue = (static_cast<double>(value[2]) - centre[2]) * normalized[2];
            double projections[4] = {static_cast<double>(green) + blue + red,
                                     (static_cast<double>(green) + red) - blue,
                                     (static_cast<double>(red) - green) + blue,
                                     (static_cast<double>(red) - green) - blue};
            for (unsigned axis = 0; axis < 4; ++axis)
                variance[axis] = projections[axis] * projections[axis] + variance[axis];
        }
        unsigned axis = 0;
        for (unsigned candidate = 1; candidate < 4; ++candidate)
            if (variance[axis] < variance[candidate])
                axis = candidate;
        if (axis & 2)
            std::swap(lower[1], upper[1]);
        if (axis & 1)
            std::swap(lower[2], upper[2]);

        if (length >= 0.000244140625f) {
            TruncationScope rounding;
            float last = static_cast<float>(levels - 1);
            for (unsigned iteration = 0; iteration < 8; ++iteration) {
                float palette[4][3];
                for (Dword entry = 0; entry < levels; ++entry)
                    for (unsigned channel = 0; channel < 3; ++channel)
                        palette[entry][channel] =
                            static_cast<double>(upper[channel]) * upper_weight[entry] +
                            static_cast<double>(lower[channel]) * lower_weight[entry];

                double delta[3];
                for (unsigned channel = 0; channel < 3; ++channel)
                    delta[channel] = static_cast<double>(upper[channel]) - lower[channel];
                delta[1] = static_cast<float>(delta[1]);
                double squared = delta[2] * delta[2] + delta[1] * delta[1] + delta[0] * delta[0];
                if (squared < 0.000244140625)
                    break;
                double scale = static_cast<double>(last) / squared;
                float direction[3];
                for (unsigned channel = 0; channel < 3; ++channel)
                    direction[channel] = scale * delta[channel];
                float lower_gradient[3] = {}, upper_gradient[3] = {};
                float lower_curvature = 0, upper_curvature = 0;
                for (unsigned pixel = 0; pixel < 16; ++pixel) {
                    const auto *value = reinterpret_cast<const float *>(samples + pixel);
                    double position_value =
                        (static_cast<double>(value[1]) - lower[1]) * direction[1] +
                        (static_cast<double>(value[0]) - lower[0]) * direction[0] +
                        (static_cast<double>(value[2]) - lower[2]) * direction[2];
                    float position = position_value;
                    int selected = static_cast<int>(levels - 1);
                    if (position_value < last) {
                        float biased = static_cast<double>(position) + 0.5;
                        selected = static_cast<int>(biased);
                    }
                    double a = static_cast<double>(lower_weight[selected]) * 0.125;
                    double b = static_cast<double>(upper_weight[selected]) * 0.125;
                    lower_curvature = a * lower_weight[selected] + lower_curvature;
                    upper_curvature = b * upper_weight[selected] + upper_curvature;
                    float green_residual = static_cast<double>(palette[selected][1]) - value[1];
                    for (unsigned channel = 0; channel < 3; ++channel) {
                        double residual =
                            channel == 1
                                ? green_residual
                                : static_cast<double>(palette[selected][channel]) - value[channel];
                        lower_gradient[channel] = a * residual + lower_gradient[channel];
                        upper_gradient[channel] = b * residual + upper_gradient[channel];
                    }
                }
                if (lower_curvature > 0) {
                    double step = -1.0 / static_cast<double>(lower_curvature);
                    for (unsigned channel = 0; channel < 3; ++channel)
                        lower[channel] =
                            static_cast<double>(lower_gradient[channel]) * step + lower[channel];
                }
                if (upper_curvature > 0) {
                    double step = -1.0 / static_cast<double>(upper_curvature);
                    for (unsigned channel = 0; channel < 3; ++channel)
                        upper[channel] =
                            static_cast<double>(upper_gradient[channel]) * step + upper[channel];
                }
                bool settled = true;
                for (unsigned channel = 0; channel < 3; ++channel)
                    settled =
                        settled &&
                        static_cast<double>(lower_gradient[channel]) * lower_gradient[channel] <
                            0.0000152587890625 &&
                        static_cast<double>(upper_gradient[channel]) * upper_gradient[channel] <
                            0.0000152587890625;
                if (settled)
                    break;
            }
        }
    }
    for (unsigned channel = 0; channel < 3; ++channel) {
        lower_out[channel] = lower[channel];
        upper_out[channel] = upper[channel];
    }
}

} // namespace nfs::graphics::surfaces::pixels
