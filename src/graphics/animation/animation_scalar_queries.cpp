#include "animation_channel_values.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
// The first time word is ignored by this saved method.
void __fastcall advance_angle_animation(void *owner, void *, Dword, Dword time_word,
                                        Dword output_word) {
    auto resource = at<void *>(owner, 12);
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    int last = int(at<std::uint16_t>(resource, 4)) - 1;
    float time = channel_float(time_word);
    if (at<Byte>(resource, 8) & 2) {
        if (time < 0) {
            int cycles = channel_frame(X(time) / Extended::integer(last, format));
            int product = int(Dword(cycles) * Dword(last));
            time = float(Extended::integer(last, format) -
                         (X(time) - Extended::integer(product, format)));
        } else if (float(last) < time) {
            auto difference = X(time) - float(last);
            int cycles = channel_frame(difference / X(float(last)));
            int product = int(Dword(cycles) * Dword(last));
            time = float(X(float(difference)) - Extended::integer(product, format));
        }
    }
    int rate = at<Byte>(owner, 21);
    int index = channel_frame(time) / rate;
    float fraction = float((X(time) - Extended::integer(int(Dword(rate) * Dword(index)), format)) /
                           Extended::integer(rate, format));
    auto output = reinterpret_cast<float *>(output_word);
    unsigned padding = at<Byte>(resource, 9) < 2 ? 2 : at<Byte>(resource, 9);
    float value = float(Extended::integer(at<Byte>(resource, 10 + padding + Dword(index)), format) *
                            1.41176474094390869140625f -
                        180.0f);
    *output = value;
    padding = at<Byte>(resource, 9) < 2 ? 2 : at<Byte>(resource, 9);
    if (last / at<Byte>(owner, 21) + 1 <= index) {
        auto previous = Extended::integer(at<Byte>(resource, 9 + padding + Dword(index)), format) *
                            1.41176474094390869140625f -
                        180.0f;
        *output = float((X(fraction) + 1.0f) * value - previous * fraction);
    } else {
        auto next = Extended::integer(at<Byte>(resource, 11 + padding + Dword(index)), format) *
                        1.41176474094390869140625f -
                    180.0f;
        *output = float((X(1) - fraction) * value + next * fraction);
    }
}

Dword __fastcall query_transition_crossing(void *owner, void *, Dword request_word,
                                          Dword output_word) {
    auto request = reinterpret_cast<const float *>(request_word);
    auto output = reinterpret_cast<float *>(output_word);
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto duration = (X(at<float>(owner, 68)) - at<float>(owner, 64)) * at<float>(owner, 40) +
                    at<float>(owner, 64);
    int limit = channel_frame(duration + duration);
    float target = request[0], direction = request[1];
    if (0 < request[2] && request[2] < float(limit))
        limit = int(Dword(channel_frame(request[2])) + 1);
    float current = 0;
    world::invoke(owner, 7, 0.0f, &current);
    float previous = current;
    float best_distance = float(X(target) - current);
    if (best_distance < 0)
        best_distance = float(X(best_distance) * -1.0f);
    int best = 0;
    for (int index = 1; index < limit; ++index) {
        world::invoke(owner, 7, float(index), &current);
        if (!std::isnan(previous) && !std::isnan(target) && !std::isnan(current) &&
            previous <= target && target <= current) {
            auto delta = X(current) - previous;
            auto directed = X(direction) * delta;
            if (!std::isnan(float(directed)) && !(directed < X(0))) {
                if (!std::isnan(float(delta)) && !(delta < X(0)) && !(X(0) < delta))
                    *output = float(Extended::integer(index - 1, format) + .5f);
                else
                    *output = float((X(target) - previous) / delta +
                                    Extended::integer(index - 1, format));
                return 1;
            }
        }
        auto distance = X(target) - current;
        if (distance < X(0))
            distance = -distance;
        if (distance < X(best_distance)) {
            best_distance = float(distance);
            best = index;
        }
        previous = current;
    }
    *output = float(best);
    return 1;
}
} // namespace nfs::game::world::actor_resources::animation_runtime
