#include "animation_channel_values.hpp"
namespace nfs::game::world::actor_resources::animation_runtime {
// Keep the existing frame conversion in its MSVC owner; the boundary is scalar.
int delta_frame_from_time(float time) {
    return channel_frame(time);
}

void decode_delta_values(void *stream, Dword first, Dword count, int previous, int target,
                         const float *retained, float *output) {
    if (previous == target)
        return;
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto *coefficients = reinterpret_cast<float *>(offset(stream, 4 + first * 12));
    if (target < previous || !target || previous == -1) {
        for (int index = 0; index < int(count); ++index)
            output[index] = coefficients[Dword(index) * 3 + 2];
        previous = 0;
    } else if (output != retained) {
        for (int index = 0; index < int(count); ++index)
            output[index] = retained[index];
    }
    if (!target)
        return;
    Dword total = at<std::uint16_t>(stream, 0), header = total * 12 + 4;
    Byte bits = at<Byte>(stream, 2);
    auto add = [&](unsigned index, Dword value, bool retained_before_offset = false) {
        auto product = Extended::integer(value, format) * coefficients[index * 3 + 1];
        output[index] =
            float(retained_before_offset ? product + output[index] + coefficients[index * 3]
                                         : product + coefficients[index * 3] + output[index]);
    };
    if (bits == 4) {
        Byte *cursor = offset(stream, first + ((total + 1) >> 1) * Dword(previous) + header);
        Dword paired_limit = count - 1;
        for (int frame = previous; frame < target; ++frame) {
            unsigned index = 0;
            if (!(first & 1)) {
                while (int(index) < int(paired_limit)) {
                    Byte packed = *cursor++;
                    add(index, packed & 15);
                    add(index + 1, packed >> 4);
                    index += 2;
                }
                if (count & 1)
                    add(index, *cursor++ & 15);
            } else {
                if (int(paired_limit) > 0) {
                    add(0, *cursor++ >> 4, true);
                    index = 1;
                    while (int(index) < int(paired_limit)) {
                        Byte packed = *cursor++;
                        add(index, packed & 15);
                        add(index + 1, packed >> 4);
                        index += 2;
                    }
                }
                if (paired_limit & 1)
                    add(index, *cursor++ & 15);
            }
        }
    } else if (bits == 8) {
        Byte *cursor = offset(stream, first + total * Dword(previous) + header);
        for (int frame = previous; frame < target; ++frame)
            for (int index = 0; index < int(count); ++index)
                add(Dword(index), *cursor++);
    } else if (bits == 16) {
        Byte *cursor = offset(stream, header + (total * Dword(previous) + first) * 2);
        for (int frame = previous; frame < target; ++frame)
            for (int index = 0; index < int(count); ++index) {
                Dword value = at<std::uint16_t>(cursor, 0);
                cursor += 2;
                add(Dword(index), value);
            }
    }
}

void interpolate_delta_values(void *stream, Dword first, int count, int previous, int target,
                              const float *retained, float *output, int width,
                              const std::uint16_t *destinations, float weight) {
    if (previous == target)
        return;
    auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return Extended(value, format); };
    auto *coefficients = reinterpret_cast<float *>(offset(stream, 4 + first * 12));
    int groups = count / width;
    if (!target || previous == -1 || target < previous) {
        unsigned component = 0;
        for (int group = 0; group < groups; ++group)
            for (int axis = 0; axis < width; ++axis, ++component)
                output[destinations[group] + axis] = coefficients[component * 3 + 2];
        previous = 0;
    } else {
        unsigned component = 0;
        for (int group = 0; group < groups; ++group)
            for (int axis = 0; axis < width; ++axis, ++component)
                output[destinations[group] + axis] = retained[component];
    }
    if (!target)
        return;
    Dword total = at<std::uint16_t>(stream, 0), header = total * 12 + 4;
    Byte bits = at<Byte>(stream, 2);
    Byte *cursor;
    if (bits == 4)
        cursor = offset(stream, header + ((total + 1) / 2) * Dword(previous));
    else if (bits == 8)
        cursor = offset(stream, header + total * Dword(previous));
    else if (bits == 16)
        cursor = offset(stream, header + total * Dword(previous) * 2);
    else
        return;
    bool low_nibble = !(first & 4294967294u);
    for (int frame = previous; frame < target; ++frame) {
        unsigned component = 0;
        for (int group = 0; group < groups; ++group) {
            for (int axis = 0; axis < width; ++axis, ++component) {
                Dword value;
                if (bits == 4) {
                    if (low_nibble)
                        value = *cursor & 15;
                    else
                        value = *cursor++ >> 4;
                    low_nibble = !low_nibble;
                } else if (bits == 8)
                    value = *cursor++;
                else {
                    value = at<std::uint16_t>(cursor, 0);
                    cursor += 2;
                }
                float &destination = output[destinations[group] + axis];
                destination =
                    float((Extended::integer(value, format) * coefficients[component * 3 + 1] +
                           coefficients[component * 3]) *
                              weight +
                          destination);
            }
        }
        if (bits == 4 && !low_nibble) {
            ++cursor;
            low_nibble = true;
        }
    }
}
} // namespace nfs::game::world::actor_resources::animation_runtime
