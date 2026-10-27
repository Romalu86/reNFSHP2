#include "frontend_application_widgets.hpp"
#include <cstdint>
namespace nfs::application::frontend {
namespace {
int round_fixed_product(int left, int right) {
    const std::int64_t product = std::int64_t(left) * right;
    return int(Dword(std::uint64_t(product) >> 16) + Dword((std::uint64_t(product) >> 15) & 1));
}
int component(int value) {
    return value < 0 ? 0 : value > 255 ? 255 : value;
}
} // namespace

static Dword rgb_to_hsv(Dword colour) {
    const int red = int((colour >> 16) & 255), green = int((colour >> 8) & 255),
              blue = int(colour & 255);
    const int maximum = std::max(red, std::max(green, blue));
    const int minimum = std::min(red, std::min(green, blue));
    const int range = maximum - minimum;
    int hue = 0, saturation = 0;
    if (maximum && range) {
        saturation = (range << 8) / maximum;
        int sector;
        if (red == maximum)
            sector = int((std::int64_t(green - blue) * 65536) / range);
        else if (green == maximum)
            sector = int((std::int64_t(blue - red) * 65536) / range) + 131072;
        else
            sector = int((std::int64_t(red - green) * 65536) / range) + 262144;
        hue = sector / 6;
        if (hue < 0)
            hue += 65536;
    }
    hue = component(round_fixed_product(256, hue));
    return (colour & 4278190080u) | (Dword(hue) << 16) | (Dword(component(saturation)) << 8) |
           Dword(component(maximum));
}

static Dword hsv_to_rgb(Dword colour) {
    const int hue = int((colour >> 16) & 255), saturation = int((colour >> 8) & 255),
              value = int(colour & 255);
    int red, green, blue;
    if (hue <= 0) {
        red = green = blue = 0;
    } else if (saturation <= 0) {
        red = green = blue = value;
    } else {
        const int sector = (hue * 6) >> 8;
        const int fraction = ((hue * 6) - (sector << 8)) << 8;
        const int low = ((256 - saturation) * value) >> 8;
        const int falling = ((256 - round_fixed_product(saturation, fraction)) * value) >> 8;
        const int rising = ((256 - round_fixed_product(saturation, 65536 - fraction)) * value) >> 8;
        switch (sector) {
        case 0:
            red = value;
            green = rising;
            blue = low;
            break;
        case 1:
            red = falling;
            green = value;
            blue = low;
            break;
        case 2:
            red = low;
            green = value;
            blue = rising;
            break;
        case 3:
            red = low;
            green = falling;
            blue = value;
            break;
        case 4:
            red = rising;
            green = low;
            blue = value;
            break;
        case 5:
            red = value;
            green = low;
            blue = falling;
            break;
        default:
            red = hue;
            green = saturation;
            blue = value;
            break;
        }
        red = component(red);
        green = component(green);
        blue = component(blue);
    }
    return (colour & 4278190080u) | (Dword(red) << 16) | (Dword(green) << 8) | Dword(blue);
}

Dword cycle_ticker_hue(Dword colour, int amount) {
    const Dword first = rgb_to_hsv(colour);
    const auto next = std::uint8_t(std::uint8_t(first >> 16) + std::uint8_t(amount));
    const Dword second = rgb_to_hsv(colour);
    return hsv_to_rgb((second & 4278255615u) | (Dword(next) << 16));
}
} // namespace nfs::application::frontend
