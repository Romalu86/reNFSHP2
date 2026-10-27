#include "nfs/graphics/textures/surface_filters.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::graphics::surfaces {
using pixels::Colour;
// The allocation carries a vector count.
static Accumulator *allocate_accumulators(Dword count) {
    auto *storage = static_cast<Dword *>(gui::application_allocate(count * 12 + 4));
    if (!storage)
        return nullptr;
    *storage = count;
    auto *out = reinterpret_cast<Accumulator *>(storage + 1);
    for (Dword i = 0; i < count; ++i)
        out[i] = {nullptr, nullptr, 0};
    return out;
}
static void release_accumulators(Accumulator *cells) {
    if (cells) {
        Dword count = reinterpret_cast<Dword *>(cells)[-1];
        while (count)
            gui::application_release(cells[--count].values);
        gui::application_release(reinterpret_cast<Dword *>(cells) - 1);
    }
}
static Dword *next(Dword *head) {
    return reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(head) + *head);
}
static Contribution *begin(Dword *head) {
    return reinterpret_cast<Contribution *>(head + 1);
}
static Contribution *end(Dword *head) {
    return reinterpret_cast<Contribution *>(next(head));
}
static void read_row(Dword *c, Dword y, Dword z, Colour *values) {
    using Call = void(__thiscall *)(Dword *, Dword, Dword, Colour *);
    reinterpret_cast<const Call *>(c[0])[1](c, y, z, values);
}
static void write_row(Dword *c, Dword y, Dword z, Colour *values) {
    using Call = void(__thiscall *)(Dword *, Dword, Dword, Colour *);
    reinterpret_cast<const Call *>(c[0])[2](c, y, z, values);
}
static void add(Colour &destination, const Colour &source, double weight) {
    auto *out = reinterpret_cast<float *>(&destination);
    const auto *in = reinterpret_cast<const float *>(&source);
    for (unsigned channel = 0; channel < 4; ++channel)
        out[channel] = static_cast<double>(weight) * in[channel] + out[channel];
}
static void references(Accumulator *cells, Dword *weights) {
    Dword *limit = reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(weights) + weights[0]);
    for (Dword *head = weights + 1; head < limit; head = next(head))
        for (auto *item = begin(head); item < end(head); ++item)
            ++cells[item->index].references;
}
static bool acquire(Accumulator &cell, Accumulator *&available, Dword count) {
    if (!cell.values) {
        if (available) {
            cell.values = available->values;
            available->values = nullptr;
            available = available->next;
        } else {
            cell.values = static_cast<Colour *>(gui::application_allocate(count * 16));
            if (!cell.values)
                return false;
        }
        std::memset(cell.values, 0, count * 16);
    }
    return true;
}
// Scatter complete source rows into lazily allocated targets.
int triangle_2d(Transfer *object) {
    Dword *s = object->source;
    Dword *d = object->destination;
    if (s[1048] != 1 || d[1048] != 1)
        return static_cast<int>(2147500037u);
    Dword *xs = triangle_weights(s[1046], d[1046], ~(object->filter >> 16) & 1);
    Dword *ys = nullptr;
    Accumulator *cells = nullptr;
    Accumulator *available = nullptr;
    Colour *input = nullptr;
    int result = static_cast<int>(2147500037u);
    if (xs) {
        ys = triangle_weights(s[1047], d[1047], ~(object->filter >> 17) & 1);
        if (ys) {
            result = static_cast<int>(2147942414u);
            cells = allocate_accumulators(d[1047]);
            if (cells) {
                input = static_cast<Colour *>(gui::application_allocate(s[1046] * 16));
                if (input) {
                    references(cells, ys);
                    result = 0;
                    Dword *yhead = ys + 1;
                    Dword *ylimit =
                        reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(ys) + ys[0]);
                    Dword y = 0;
                    for (; yhead < ylimit; yhead = next(yhead), ++y) {
                        for (auto *target = begin(yhead); target < end(yhead); ++target)
                            if (!acquire(cells[target->index], available, d[1046])) {
                                result = static_cast<int>(2147942414u);
                                goto cleanup;
                            }
                        read_row(s, y, 0, input);
                        Dword *xhead = xs + 1;
                        Dword *xlimit =
                            reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(xs) + xs[0]);
                        Dword x = 0;
                        for (; xhead < xlimit; xhead = next(xhead), ++x)
                            for (auto *ty = begin(yhead); ty < end(yhead); ++ty)
                                for (auto *tx = begin(xhead); tx < end(xhead); ++tx) {
                                    double weight = static_cast<double>(tx->weight) * ty->weight;
                                    add(cells[ty->index].values[tx->index], input[x], weight);
                                }
                        for (auto *target = begin(yhead); target < end(yhead); ++target) {
                            auto &cell = cells[target->index];
                            if (!--cell.references) {
                                write_row(d, target->index, 0, cell.values);
                                cell.next = available;
                                available = &cell;
                            }
                        }
                    }
                }
            }
        }
    }
cleanup:
    release_accumulators(cells);
    gui::application_release(ys);
    gui::application_release(xs);
    gui::application_release(input);
    gui::application_release(nullptr);
    return result;
}
// Target slices remain live until their last contributing source slice.
int triangle_3d(Transfer *object) {
    Dword *s = object->source;
    Dword *d = object->destination;
    Dword *xs = triangle_weights(s[1046], d[1046], ~(object->filter >> 16) & 1);
    Dword *ys = nullptr;
    Dword *zs = nullptr;
    Accumulator *cells = nullptr;
    Accumulator *available = nullptr;
    Colour *input = nullptr;
    int result = static_cast<int>(2147500037u);
    if (xs) {
        ys = triangle_weights(s[1047], d[1047], ~(object->filter >> 17) & 1);
        if (ys) {
            zs = triangle_weights(s[1048], d[1048], ~(object->filter >> 18) & 1);
            if (zs) {
                result = static_cast<int>(2147942414u);
                cells = allocate_accumulators(d[1048]);
                if (cells) {
                    input = static_cast<Colour *>(gui::application_allocate(s[1046] * 16));
                    if (input) {
                        references(cells, zs);
                        result = 0;
                        Dword *zlimit =
                            reinterpret_cast<Dword *>(reinterpret_cast<std::uint8_t *>(zs) + zs[0]);
                        Dword z = 0;
                        for (Dword *zhead = zs + 1; zhead < zlimit; zhead = next(zhead), ++z) {
                            for (auto *target = begin(zhead); target < end(zhead); ++target)
                                if (!acquire(cells[target->index], available,
                                             d[1046] * d[1047])) {
                                    result = static_cast<int>(2147942414u);
                                    goto cleanup;
                                }
                            Dword *ylimit = reinterpret_cast<Dword *>(
                                reinterpret_cast<std::uint8_t *>(ys) + ys[0]);
                            Dword y = 0;
                            for (Dword *yhead = ys + 1; yhead < ylimit; yhead = next(yhead), ++y) {
                                read_row(s, y, z, input);
                                Dword *xlimit = reinterpret_cast<Dword *>(
                                    reinterpret_cast<std::uint8_t *>(xs) + xs[0]);
                                Dword x = 0;
                                for (Dword *xhead = xs + 1; xhead < xlimit; xhead = next(xhead), ++x)
                                    for (auto *tz = begin(zhead); tz < end(zhead); ++tz)
                                        for (auto *ty = begin(yhead); ty < end(yhead); ++ty)
                                            for (auto *tx = begin(xhead); tx < end(xhead); ++tx) {
                                                double weight = static_cast<double>(tx->weight) *
                                                                ty->weight * tz->weight;
                                                add(cells[tz->index]
                                                        .values[d[1046] * ty->index + tx->index],
                                                    input[x], weight);
                                            }
                            }
                            for (auto *target = begin(zhead); target < end(zhead); ++target) {
                                auto &cell = cells[target->index];
                                if (!--cell.references) {
                                    for (Dword row = 0; row < d[1047]; ++row)
                                        write_row(d, row, target->index,
                                                  cell.values + d[1046] * row);
                                    cell.next = available;
                                    available = &cell;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
cleanup:
    release_accumulators(cells);
    gui::application_release(zs);
    gui::application_release(ys);
    gui::application_release(xs);
    gui::application_release(input);
    gui::application_release(nullptr);
    return result;
}
} // namespace nfs::graphics::surfaces
