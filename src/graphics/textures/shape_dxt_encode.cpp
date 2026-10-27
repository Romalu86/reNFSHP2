#include "nfs/graphics/textures/shape_conversion.hpp"
#include <cstring>

namespace nfs::graphics::textures::shape {
namespace {
struct QuantizedColour {
    int red, green, blue;
    bool transparent;
};
Dword packed_colour(const QuantizedColour &colour) {
    return Dword((colour.red << 11) | (colour.green << 5) | colour.blue);
}
int difference(int left, int right) {
    return left < right ? right - left : left - right;
}
int distance(const QuantizedColour &left, const QuantizedColour &right) {
    return difference(left.red, right.red) + difference(left.green, right.green) +
           difference(left.blue, right.blue);
}
QuantizedColour interpolate(const QuantizedColour &left, const QuantizedColour &right, bool half) {
    if (half)
        return {(left.red + right.red) / 2, (left.green + right.green) / 2,
                (left.blue + right.blue) / 2, left.transparent || right.transparent};
    return {((left.red * 2 + right.red) * 43690 + 21845) >> 17,
            ((left.green * 2 + right.green) * 43690 + 21845) >> 17,
            ((left.blue * 2 + right.blue) * 43690 + 21845) >> 17, left.transparent};
}
} // namespace

// This integer palette search belongs to SHAPE. Its ordering, distance ties and early exits
// differ from the D3DX floating-point fitter. Selectors outside a partial tile's width remain
// uninitialized.
 
void encode_dxt_tile(const void *source, void *destination, int source_width, int width, int height,
                     int format, int search_scale, int search_precision, int error_threshold) {
    const auto *input = static_cast<const Dword *>(source);
    auto *output = static_cast<std::uint16_t *>(destination);
    QuantizedColour colours[16];
    unsigned frequencies[16];
    Byte pixel_colours[16], best_selectors[16], selectors[16];
    unsigned unique = 0, pixel_count = 0;
    bool transparent = false;
    QuantizedColour first = {}, second = {};
    Dword best_error = 4294967295u;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Dword pixel = input[y * source_width + x];
            QuantizedColour colour = {int(pixel >> 19 & 31), int(pixel >> 10 & 63),
                                      int(pixel >> 3 & 31), format == 1 && pixel < 1610612736};
            transparent |= colour.transparent;
            unsigned index = 0;
            while (index < unique) {
                if (colour.transparent ? colours[index].transparent
                                       : (!colours[index].transparent &&
                                          packed_colour(colours[index]) == packed_colour(colour)))
                    break;
                ++index;
            }
            if (index == unique) {
                colours[unique] = colour;
                frequencies[unique++] = 0;
            }
            ++frequencies[index];
            pixel_colours[pixel_count++] = Byte(index);
        }
    }
    bool endpoints_ready = false;
    if (unique == 1) {
        if (transparent) {
            second = {1, 1, 1, false};
            best_selectors[0] = 3;
        } else {
            first = colours[0];
            best_selectors[0] = 0;
        }
        endpoints_ready = true;
    } else if (unique == 2) {
        if (transparent) {
            unsigned visible = colours[0].transparent ? 1 : 0;
            second = colours[visible];
            best_selectors[visible] = 1;
            best_selectors[visible ^ 1] = 3;
        } else {
            first = colours[0];
            second = colours[1];
            if (packed_colour(first) < packed_colour(second)) {
                first = colours[1];
                second = colours[0];
            }
            bool kept_order = packed_colour(first) == packed_colour(colours[0]);
            best_selectors[0] = kept_order ? 0 : 1;
            best_selectors[1] = kept_order ? 1 : 0;
        }
        endpoints_ready = true;
    }
    Dword search_extent = 0;
    if (!endpoints_ready) {
        // Preserve the running-value recurrence used by the palette error metric.
         
        for (unsigned i = 0; i < unique; ++i)
            search_extent += difference(colours[i].red * 8, int(search_extent)) +
                             colours[i].green * 4 + colours[i].blue * 8;
        for (unsigned i = 0; i + 1 < unique; ++i) {
            for (unsigned j = i + 1; j < unique; ++j) {
                QuantizedColour high = colours[i], low = colours[j];
                if (high.transparent || low.transparent)
                    continue;
                if (packed_colour(high) < packed_colour(low)) {
                    high = colours[j];
                    low = colours[i];
                }
                QuantizedColour palette[4] = {high, low, interpolate(high, low, transparent),
                                              interpolate(low, high, false)};
                if (transparent) {
                    palette[0] = low;
                    palette[1] = high;
                }
                Dword error = 0;
                for (unsigned entry = 0; entry < unique; ++entry) {
                    const auto &colour = colours[entry];
                    unsigned selected;
                    if (transparent) {
                        if (colour.transparent) {
                            selectors[entry] = 3;
                            continue;
                        }
                        selected = 0;
                        int nearest = distance(colour, palette[0]);
                        int candidate = distance(colour, palette[1]);
                        if (candidate < nearest) {
                            selected = 1;
                            nearest = candidate;
                        }
                        if (distance(colour, palette[2]) < nearest)
                            selected = 2;
                    } else {
                        // The branch tree gives a different tie priority than
                        // a linear minimum over four palette entries.
                        int d0 = distance(colour, palette[0]), d2 = distance(colour, palette[2]);
                        selected = 0;
                        if (d2 < d0) {
                            int d3 = distance(colour, palette[3]);
                            if (d2 < d3)
                                selected = 2;
                            else
                                selected = d3 < distance(colour, palette[1]) ? 3 : 1;
                        }
                    }
                    selectors[entry] = Byte(selected);
                    const auto &fitted = palette[selected];
                    int red = (colour.red - fitted.red) * 8;
                    int green = (colour.green - fitted.green) * 4;
                    int blue = (colour.blue - fitted.blue) * 8;
                    error += Dword(red * red + green * green + blue * blue) * frequencies[entry];
                }
                if (error < best_error) {
                    first = palette[0];
                    second = palette[1];
                    best_error = error;
                    std::memcpy(best_selectors, selectors, unique);
                }
                if (Dword(search_scale) < search_extent && Dword(search_precision) < unique &&
                    best_error < Dword(error_threshold))
                    break;
            }
        }
    }
    if (format == 3) {
        for (int y = 0; y < height; ++y) {
            const Dword *row = input + y * source_width;
            output[y] = std::uint16_t((row[0] >> 28) | (row[1] >> 24 & 240) |
                                      (row[2] >> 20 & 3840) | (row[3] >> 16 & 61440));
        }
        output += 4;
    }
    output[0] = static_cast<std::uint16_t>(packed_colour(first));
    output[1] = static_cast<std::uint16_t>(packed_colour(second));
    Dword bits = 0;
    for (int row = 0; row < width * height; row += width) {
        Byte packed =
            Byte(best_selectors[pixel_colours[row]] | best_selectors[pixel_colours[row + 1]] << 2 |
                 best_selectors[pixel_colours[row + 2]] << 4 |
                 best_selectors[pixel_colours[row + 3]] << 6);
        bits = bits >> 8 | Dword(packed) << 24;
    }
    *reinterpret_cast<Dword *>(output + 2) = bits >> ((32 - height * 8) & 31);
}
} // namespace nfs::graphics::textures::shape
