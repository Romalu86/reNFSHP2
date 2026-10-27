#include "animation_transition_runtime.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
namespace {
Dword blend_displacement(void *owner, unsigned first_child, unsigned second_child,
                        unsigned weight_field, float first_time, float second_time, float *out) {
    float first[2], second[2];
    if (!Byte(world::invoke<Dword>(at<void *>(owner, first_child), 8, channel_word(first_time),
                                  first)))
        return 0;
    float weight = at<float>(owner, weight_field);
    if (weight != 0) {
        if (!Byte(world::invoke<Dword>(at<void *>(owner, second_child), 8, channel_word(second_time),
                                      second)))
            return 0;
        auto format = kernel::numeric::current_format();
        auto X = [&](float v) { return Extended(v, format); };
        float retained_first_weight = float(X(1) - at<float>(owner, weight_field));
        out[0] = float(X(first[0]) * retained_first_weight +
                       X(second[0]) * at<float>(owner, weight_field));
        out[1] = float(X(first[1]) * retained_first_weight +
                       X(second[1]) * at<float>(owner, weight_field));
        auto length = duration_displacement_length(out);
        float stored_length = float(length);
        if (std::isnan(float(length)) || X(0) < length || length < X(0)) {
            float second_length =
                float(duration_displacement_length(second) * at<float>(owner, weight_field));
            auto scale =
                (duration_displacement_length(first) * retained_first_weight + second_length) /
                X(stored_length);
            out[0] = float(scale * out[0]);
            out[1] = float(scale * out[1]);
        }
    } else {
        std::memcpy(out, first, 8);
    }
    return 1;
}
} // namespace

Dword __fastcall sample_duration_displacement(void *owner, void *, float first_time,
                                             float second_time, float *out) {
    return blend_displacement(owner, 16, 20, 24, first_time, second_time, out);
}

Dword __fastcall sample_frame_displacement(void *owner, void *, float first_time, float second_time,
                                          float *out) {
    return blend_displacement(owner, 32, 36, 40, first_time, second_time, out);
}
} // namespace nfs::game::world::actor_resources::animation_runtime
