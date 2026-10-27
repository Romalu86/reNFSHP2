#include "nfs/graphics/textures/surface_filters.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::graphics::surfaces {
using pixels::Colour;
template <class T> static void row(Dword *codec, unsigned slot, Dword y, Dword z, T *values) {
    using Call = void(__thiscall *)(Dword *, Dword, Dword, T *);
    reinterpret_cast<const Call *>(codec[0])[slot](codec, y, z, values);
}
static bool half_size(Dword source, Dword destination) {
    return destination == (source >> 1) || (destination == 1 && source == 1);
}
// Fast packed paths precede float-vector averaging.
int half_2d(Transfer *object) {
    Dword *s = object->source;
    Dword *d = object->destination;
    if (static_cast<std::uint8_t>(object->filter) != 5 || !half_size(s[1046], d[1046]) ||
        !half_size(s[1047], d[1047]) || s[1048] != 1 || d[1048] != 1)
        return static_cast<int>(2147500037u);
    if (s[1046] > 1 && (s[1046] & 1)) {
        s[1046] &= ~1u;
        s[1049] -= s[1050];
    }
    if (s[1047] > 1)
        s[1047] &= ~1u;
    if (!(object->filter & 524288) && s[1] == d[1] && s[1046] > 1 && s[1047] > 1) {
        int result = packed_half(object);
        if (result >= 0)
            return 0;
    }
    auto *output = static_cast<Colour *>(gui::application_allocate(d[1046] * 16));
    if (!output)
        return static_cast<int>(2147942414u);
    auto *input =
        static_cast<Colour *>(gui::application_allocate(s[1046] * (s[1047] == 1 ? 16 : 32)));
    if (!input) {
        gui::application_release(output);
        return static_cast<int>(2147942414u);
    }
    Colour *first = input;
    Colour *second = s[1047] == 1 ? first : first + s[1046];
    Dword adjacent = s[1046] == 1 ? 0 : 1;
    for (Dword y = 0; y < d[1047]; ++y) {
        row(s, 1, y * 2, 0, first);
        if (second != first)
            row(s, 1, y * 2 + 1, 0, second);
        for (Dword x = 0; x < d[1046]; ++x) {
            auto *a = reinterpret_cast<const float *>(first + x * 2);
            auto *b = reinterpret_cast<const float *>(first + x * 2 + adjacent);
            auto *c = reinterpret_cast<const float *>(second + x * 2);
            auto *e = reinterpret_cast<const float *>(second + x * 2 + adjacent);
            auto *out = reinterpret_cast<float *>(output + x);
            for (int component = 0; component < 4; ++component) {
                double pair = static_cast<double>(b[component]) + a[component];
                if (component >= 2)
                    pair = static_cast<float>(pair);
                double three = pair + c[component];
                if (component == 0 || component == 3)
                    three = static_cast<float>(three);
                double four = three + e[component];
                if (component < 2)
                    four = static_cast<float>(four);
                out[component] = four * 0.25;
            }
        }
        row(d, 2, y, 0, output);
    }
    gui::application_release(input);
    gui::application_release(output);
    return 0;
}
// Row order: y0/z0, y0/z1, y1/z0, y1/z1.
int half_3d(Transfer *object) {
    Dword *s = object->source;
    Dword *d = object->destination;
    if (static_cast<std::uint8_t>(object->filter) != 5 || !half_size(s[1046], d[1046]) ||
        !half_size(s[1047], d[1047]) || d[1048] != (s[1048] >> 1))
        return static_cast<int>(2147500037u);
    if (s[1046] > 1 && (s[1046] & 1)) {
        s[1046] &= ~1u;
        s[1049] -= s[1050];
    }
    if (s[1047] > 1)
        s[1047] &= ~1u;
    if (s[1048] > 1)
        s[1048] &= ~1u;
    auto *output = static_cast<Colour *>(gui::application_allocate(d[1046] * 16));
    if (!output)
        return static_cast<int>(2147942414u);
    auto *input =
        static_cast<Colour *>(gui::application_allocate(s[1046] * (s[1047] == 1 ? 32 : 64)));
    if (!input) {
        gui::application_release(output);
        return static_cast<int>(2147942414u);
    }
    Colour *lines[4] = {input, input + s[1046], input + (s[1047] == 1 ? 0 : 2 * s[1046]),
                        input + (s[1047] == 1 ? s[1046] : 3 * s[1046])};
    Dword adjacent = s[1046] == 1 ? 0 : 1;
    for (Dword z = 0; z < d[1048]; ++z)
        for (Dword y = 0; y < d[1047]; ++y) {
            row(s, 1, y * 2, z * 2, lines[0]);
            if (lines[1] != lines[0])
                row(s, 1, y * 2, z * 2 + 1, lines[1]);
            if (lines[2] != lines[0])
                row(s, 1, y * 2 + 1, z * 2, lines[2]);
            if (lines[3] != lines[1] && lines[3] != lines[2])
                row(s, 1, y * 2 + 1, z * 2 + 1, lines[3]);
            for (Dword x = 0; x < d[1046]; ++x) {
                auto *out = reinterpret_cast<float *>(output + x);
                for (int channel = 0; channel < 4; ++channel) {
                    double values[8];
                    for (unsigned line = 0; line < 4; ++line) {
                        values[line * 2] =
                            reinterpret_cast<const float *>(lines[line] + x * 2)[channel];
                        values[line * 2 + 1] = reinterpret_cast<const float *>(lines[line] + x * 2 +
                                                                               adjacent)[channel];
                    }
                    double value = values[1] + values[0];
                    if (channel >= 2)
                        value = static_cast<float>(value);
                    value += values[2];
                    if (channel == 0 || channel == 3)
                        value = static_cast<float>(value);
                    value += values[3];
                    if (channel < 2)
                        value = static_cast<float>(value);
                    value += values[4];
                    if (channel < 3)
                        value = static_cast<float>(value);
                    value += values[5];
                    if (channel > 0)
                        value = static_cast<float>(value);
                    value += values[6];
                    if (channel >= 2)
                        value = static_cast<float>(value);
                    value += values[7];
                    if (channel == 0 || channel == 3)
                        value = static_cast<float>(value);
                    out[channel] = value * 0.125;
                }
            }

            row(d, 2, y, z, output);
        }
    gui::application_release(input);
    gui::application_release(output);
    return 0;
}
} // namespace nfs::graphics::surfaces
