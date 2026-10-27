#include "nfs/graphics/textures/surface_filters.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cfenv>
#include <cmath>
namespace nfs::graphics::surfaces {
// Destination and source counts are tracked separately.
LinearSample *linear_samples(Dword destination, Dword source, Dword wrap) {
    auto *samples = static_cast<LinearSample *>(gui::application_allocate(destination * 16));
    if (!samples)
        return nullptr;
    float step = static_cast<double>(source) / destination;
    int rounding = std::fegetround();
    std::fesetround(FE_TOWARDZERO);
    for (Dword i = 0; i < destination; ++i) {
        double position_value = static_cast<double>(i) * step - 0.5;
        float position = position_value;
        float lower = std::floor(position_value);
        Dword first = static_cast<Dword>(static_cast<int>(lower)), second = first + 1;
        if (static_cast<int>(first) < 0)
            first = wrap ? source - 1 : 0;
        if (second >= source)
            second = wrap ? 0 : source - 1;
        double weight = 1.0 - (static_cast<double>(position) - lower);
        samples[i] = {first, static_cast<float>(weight), second, static_cast<float>(1.0 - weight)};
    }
    std::fesetround(rounding);
    return samples;
}
// A variable-length list scatters each source cell to targets.
// Header word = total bytes; each cell begins with its own relative byte size.
Dword *triangle_weights(Dword source, Dword destination, Dword wrap) {
    if (!source || !destination)
        return nullptr;
    float target_count = static_cast<float>(destination), source_count = static_cast<float>(source),
          step = static_cast<double>(target_count) / source_count,
          half = 0.5 / (static_cast<double>(target_count) / source_count);
    Dword allocation = 16;
    for (Dword i = 0; i < source; ++i)
        allocation +=
            16 * static_cast<Dword>(static_cast<int>(static_cast<double>(step) + (wrap != 0) + 1)) +
            12;
    auto *table = static_cast<Dword *>(gui::application_allocate(allocation));
    if (!table)
        return nullptr;
    Dword cursor = 4;
    int previous = 0;
    float accumulated = 0;
    for (Dword i = 0; i < source; ++i) {
        Dword head = cursor;
        cursor += 4;
        for (Dword side = 0; side < 2; ++side) {
            float cell = static_cast<double>(side) + static_cast<float>(i) - 0.5;
            double lower_value = static_cast<double>(cell) * step;
            float lower = lower_value;
            float upper = static_cast<double>(step) + lower_value;
            if (!wrap) {
                if (lower_value < 0) {
                    lower = 0;
                    lower_value = 0;
                }
                if (upper > target_count)
                    upper = target_count;
            }
            int index = static_cast<int>(std::floor(lower_value));
            for (double start = index; start < upper; start = ++index) {
                float end = static_cast<double>(index) + 1;
                int target = index < 0 ? index + static_cast<int>(destination)
                             : index >= static_cast<int>(destination)
                                 ? index - static_cast<int>(destination)
                                 : index;
                if (target != previous) {
                    if (accumulated > 0.00000999999974737875163555145263671875f) {
                        table[cursor / 4] = previous;
                        std::memcpy(table + cursor / 4 + 1, &accumulated, 4);
                        cursor += 8;
                    }
                    accumulated = 0;
                    previous = target;
                }
                if (start < lower)
                    start = lower;
                if (end > upper)
                    end = upper;
                double weight;
                if (!wrap && cell < 0)
                    weight = 1;
                else if (!wrap && static_cast<double>(cell) + 1 >= source_count)
                    weight = 0;
                else
                    weight = (static_cast<double>(end) + start) * half - cell;
                if (side)
                    weight = 1 - weight;
                accumulated = (static_cast<double>(end) - start) * weight + accumulated;
            }
        }
        if (accumulated > 0.00000999999974737875163555145263671875f) {
            table[cursor / 4] = previous;
            std::memcpy(table + cursor / 4 + 1, &accumulated, 4);
            cursor += 8;
        }
        accumulated = 0;
        table[head / 4] = cursor - head;
    }
    table[0] = cursor;
    return table;
}
} // namespace nfs::graphics::surfaces
