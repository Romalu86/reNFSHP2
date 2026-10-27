#include "nfs/graphics/textures/surface_filters.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <utility>
namespace nfs::graphics::surfaces {
using pixels::Colour;
static void read_row(Dword *c, Dword y, Dword z, Colour *values) {
    using Call = void(__thiscall *)(Dword *, Dword, Dword, Colour *);
    reinterpret_cast<const Call *>(c[0])[1](c, y, z, values);
}
static void write_row(Dword *c, Dword y, Dword z, Colour *values) {
    using Call = void(__thiscall *)(Dword *, Dword, Dword, Colour *);
    reinterpret_cast<const Call *>(c[0])[2](c, y, z, values);
}
// Keep separate RGB and alpha float-store points.
static double horizontal(const Colour *row, const LinearSample &sample, unsigned channel) {
    double first = static_cast<double>(sample.first_weight) *
                   reinterpret_cast<const float *>(row + sample.first)[channel],
           second = static_cast<double>(sample.second_weight) *
                    reinterpret_cast<const float *>(row + sample.second)[channel];
    if (channel)
        first = static_cast<float>(first);
    if (channel < 3)
        second = static_cast<float>(second);
    double sum = first + second;
    return channel < 3 ? static_cast<float>(sum) : sum;
}
static double bilinear(const Colour *first, const Colour *second, const LinearSample &x,
                       const LinearSample &y, unsigned channel) {
    double a = horizontal(first, x, channel) * y.first_weight,
           b = horizontal(second, x, channel) * y.second_weight;
    if (channel < 3) {
        a = static_cast<float>(a);
        b = static_cast<float>(b);
    }
    double sum = a + b;
    return channel < 3 ? static_cast<float>(sum) : sum;
}
// This path writes z = 0.
int linear_2d(Transfer *object) {
    if (static_cast<std::uint8_t>(object->filter) != 3)
        return static_cast<int>(2147500037u);
    Dword *s = object->source;
    Dword *d = object->destination;
    auto *xs = linear_samples(d[1046], s[1046], ~(object->filter >> 16) & 1);
    auto *ys = linear_samples(d[1047], s[1047], ~(object->filter >> 17) & 1);
    Colour *output = nullptr;
    Colour *input = nullptr;
    int result = static_cast<int>(2147942414u);
    if (xs && ys) {
        output = static_cast<Colour *>(gui::application_allocate(d[1046] * 16));
        if (output) {
            input = static_cast<Colour *>(gui::application_allocate(s[1046] * 32));
            if (input) {
                Colour *first = input;
                Colour *second = input + s[1046];
                Dword cached_first = ~0u, cached_second = ~0u;
                for (Dword y = 0; y < d[1047]; ++y) {
                    const auto &sy = ys[y];
                    if (sy.first != cached_first) {
                        if (sy.first == cached_second) {
                            std::swap(first, second);
                            cached_first = cached_second;
                            cached_second = ~0u;
                        } else {
                            cached_first = sy.first;
                            read_row(s, sy.first, 0, first);
                        }
                    }
                    if (sy.second != cached_second) {
                        cached_second = sy.second;
                        read_row(s, sy.second, 0, second);
                    }
                    for (Dword x = 0; x < d[1046]; ++x) {
                        const auto &sx = xs[x];
                        auto *out = reinterpret_cast<float *>(output + x);
                        for (unsigned channel = 0; channel < 4; ++channel)
                            out[channel] = bilinear(first, second, sx, sy, channel);
                    }
                    write_row(d, y, 0, output);
                }
                result = 0;
            }
        }
    }
    gui::application_release(xs);
    gui::application_release(ys);
    gui::application_release(output);
    gui::application_release(input);
    return result;
}
// Four cached source rows, with a pair exchange on y reuse.
int linear_3d(Transfer *object) {
    if (static_cast<std::uint8_t>(object->filter) != 3)
        return static_cast<int>(2147500037u);
    Dword *s = object->source;
    Dword *d = object->destination;
    auto *xs = linear_samples(d[1046], s[1046], ~(object->filter >> 16) & 1);
    auto *ys = linear_samples(d[1047], s[1047], ~(object->filter >> 17) & 1);
    auto *zs = linear_samples(d[1048], s[1048], ~(object->filter >> 18) & 1);
    Colour *output = nullptr;
    Colour *input = nullptr;
    int result = static_cast<int>(2147942414u);
    if (xs && ys && zs) {
        output = static_cast<Colour *>(gui::application_allocate(d[1046] * 16));
        if (output) {
            input = static_cast<Colour *>(gui::application_allocate(s[1046] * 64));
            if (input) {
                Colour *lines[4] = {input, input + s[1046], input + 2 * s[1046],
                                    input + 3 * s[1046]};
                for (Dword z = 0; z < d[1048]; ++z) {
                    Dword cached_first = ~0u, cached_second = ~0u;
                    const auto &sz = zs[z];
                    for (Dword y = 0; y < d[1047]; ++y) {
                        const auto &sy = ys[y];
                        if (sy.first != cached_first) {
                            if (sy.first == cached_second) {
                                cached_first = cached_second;
                                cached_second = ~0u;
                                std::swap(lines[0], lines[1]);
                                std::swap(lines[2], lines[3]);
                            } else {
                                cached_first = sy.first;
                                read_row(s, sy.first, sz.first, lines[0]);
                                read_row(s, sy.first, sz.second, lines[2]);
                            }
                        }
                        if (sy.second != cached_second) {
                            cached_second = sy.second;
                            read_row(s, sy.second, sz.first, lines[1]);
                            read_row(s, sy.second, sz.second, lines[3]);
                        }
                        for (Dword x = 0; x < d[1046]; ++x) {
                            const auto &sx = xs[x];
                            auto *out = reinterpret_cast<float *>(output + x);
                            for (unsigned channel = 0; channel < 4; ++channel) {
                                double front = bilinear(lines[0], lines[1], sx, sy, channel) *
                                               sz.first_weight,
                                       back = bilinear(lines[2], lines[3], sx, sy, channel) *
                                              sz.second_weight;
                                if (channel < 3) {
                                    front = static_cast<float>(front);
                                    back = static_cast<float>(back);
                                }
                                out[channel] = front + back;
                            }
                        }
                        write_row(d, y, z, output);
                    }
                }
                result = 0;
            }
        }
    }
    gui::application_release(xs);
    gui::application_release(ys);
    gui::application_release(zs);
    gui::application_release(output);
    gui::application_release(input);
    return result;
}
} // namespace nfs::graphics::surfaces
