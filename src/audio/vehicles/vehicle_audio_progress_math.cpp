#include "vehicle_audio_progress_math.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
#include <cstring>

namespace nfs::game::audio::vehicle::progress_math {
namespace {
long double weight(const Progress *progress) {
    const Progress *previous = progress->previous;
    while (previous && !previous->total) {
        progress = previous;
        previous = progress->previous;
    }
    if (!previous)
        return 1.0L;
    const auto parent_weight = weight(previous);
    return parent_weight /
           static_cast<long double>(static_cast<const volatile Progress *>(previous)->total);
}

long double fraction(const Progress *progress) {
    const int total = progress->total;
    return total ? static_cast<long double>(progress->completed) /
                       static_cast<long double>(total)
                 : 0.0L;
}
}

void weight_native80(const Progress *progress, void *output) {
    const auto value = weight(progress);
    std::memcpy(output, &value, 10);
}

void fraction_native80(const Progress *progress, void *output) {
    const auto value = fraction(progress);
    std::memcpy(output, &value, 10);
}

void combined_native80(void *output) {
    auto completed = 0.0L;
    const Progress *progress = progress_head;
    while (progress) {
        const auto portion = fraction(progress);
        const Progress *previous = progress->previous;
        auto multiplier = 1.0L;
        if (previous) {
            const int total = previous->total;
            multiplier = weight(previous);
            if (total)
                multiplier /= static_cast<long double>(total);
        }
        progress = progress->next;
        const auto contribution = multiplier * portion;
        completed = completed + contribution;
    }
    if (completed < 0.0L)
        completed = 0.0L;
    else if (completed > 1.0L)
        completed = 1.0L;
    std::memcpy(output, &completed, 10);
}

// The loading widget stores both the fraction and running sum as binary32.
float loading_completion_f32() {
    float completed = 0.0f;
    const Progress *progress = progress_head;
    while (progress) {
        const float portion = float(fraction(progress));
        const Progress *previous = progress->previous;
        auto multiplier = 1.0L;
        if (previous) {
            if (!previous->total) {
                multiplier = weight(previous);
            } else {
                multiplier = weight(previous);
                multiplier /=
                    static_cast<long double>(static_cast<const volatile Progress *>(previous)->total);
            }
        }
        const auto contribution = multiplier * portion;
        progress = progress->next;
        completed = float(contribution + completed);
    }
    if (completed < 0.0f)
        completed = 0.0f;
    else if (completed > 1.0f)
        completed = 1.0f;
    return completed;
}
}
