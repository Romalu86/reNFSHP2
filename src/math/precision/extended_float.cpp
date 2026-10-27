#include "extended_float.hpp"
#include <cstring>
#include <float.h>
namespace nfs::kernel::numeric {
namespace {
struct Wide {
    std::uint64_t low, high;
};
bool nonzero(Wide a) {
    return a.low || a.high;
}
bool less(Wide a, Wide b) {
    return a.high < b.high || (a.high == b.high && a.low < b.low);
}
Wide add(Wide a, Wide b) {
    auto low = a.low + b.low;
    return {low, a.high + b.high + (low < a.low)};
}
Wide subtract(Wide a, Wide b) {
    return {a.low - b.low, a.high - b.high - (a.low < b.low)};
}
Wide left(Wide a, unsigned n) {
    if (n >= 128)
        return {};
    if (n >= 64)
        return {0, a.low << (n - 64)};
    if (!n)
        return a;
    return {a.low << n, (a.high << n) | (a.low >> (64 - n))};
}
Wide right(Wide a, unsigned n) {
    if (n >= 128)
        return {};
    if (n >= 64)
        return {a.high >> (n - 64), 0};
    if (!n)
        return a;
    return {(a.low >> n) | (a.high << (64 - n)), a.high >> n};
}
bool bit(Wide a, unsigned n) {
    return n < 64 ? bool((a.low >> n) & 1) : n < 128 ? bool((a.high >> (n - 64)) & 1) : false;
}
bool below(Wide a, unsigned n) {
    if (n >= 128)
        return nonzero(a);
    if (n >= 64)
        return a.low || (n > 64 && (a.high & ((std::uint64_t(1) << (n - 64)) - 1)));
    return n && bool(a.low & ((std::uint64_t(1) << n) - 1));
}
Wide jam(Wide a, unsigned n) {
    Wide value = right(a, n);
    if (below(a, n))
        value.low |= 1;
    return value;
}
unsigned top(Wide a) {
    unsigned n = a.high ? 64 : 0;
    auto value = a.high ? a.high : a.low;
    while (value >> 1) {
        value >>= 1;
        ++n;
    }
    return n;
}
Wide product(std::uint64_t a, std::uint64_t b) {
    Wide result{};
    for (unsigned i = 0; i < 2; ++i)
        for (unsigned j = 0; j < 2; ++j) {
            auto p = std::uint64_t(std::uint32_t(a >> (i * 32))) * std::uint32_t(b >> (j * 32));
            result = add(result, left({p, 0}, (i + j) * 32));
        }
    return result;
}
bool increment(bool sign, unsigned mode, bool guard, bool sticky, std::uint64_t last) {
    if (mode == 0)
        return guard && (sticky || (last & 1));
    if (mode == 1)
        return sign && (guard || sticky);
    if (mode == 2)
        return !sign && (guard || sticky);
    return false;
}
} // namespace
Format current_format() {
    const unsigned control = _control87(0, 0);
    const unsigned precision = control & _MCW_PC;
    return {precision == _PC_24 ? 24u : precision == _PC_53 ? 53u : 64u, (control & _MCW_RC) >> 8};
}
Extended::Extended(float value, Format mode) : format(mode) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    negative = bool(bits >> 31);
    const unsigned stored = (bits >> 23) & 255;
    const auto fraction = bits & 8388607;
    if (stored == 255) {
        kind = fraction ? 2 : 1;
        significand = std::uint64_t(fraction) << 40;
        if (kind == 2)
            significand |= 9223372036854775808ull;
        return;
    }
    if (!stored) {
        if (!fraction)
            return;
        significand = std::uint64_t(fraction) << 40;
        exponent = -126;
        while (!(significand >> 63)) {
            significand <<= 1;
            --exponent;
        }
    } else {
        exponent = int(stored) - 127;
        significand = std::uint64_t(fraction | 8388608) << 40;
    }
}
Extended Extended::from_double(double value, Format mode) {
    Extended result(0.f, mode);
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    result.negative = bool(bits >> 63);
    const unsigned stored = unsigned((bits >> 52) & 2047);
    const auto fraction = bits & 4503599627370495ull;
    if (stored == 2047) {
        result.kind = fraction ? 2 : 1;
        result.significand = fraction << 11;
        if (result.kind == 2)
            result.significand |= 9223372036854775808ull;
    } else if (stored) {
        result.exponent = int(stored) - 1023;
        result.significand = (fraction | 4503599627370496ull) << 11;
    } else if (fraction) {
        result.exponent = -1022;
        result.significand = fraction << 11;
        while (!(result.significand >> 63)) {
            result.significand <<= 1;
            --result.exponent;
        }
    }
    return result;
}
Extended Extended::from_native80(const void *value, Format mode) {
    Extended result(0.f, mode);
    std::uint64_t stored_significand;
    std::uint16_t sign_exponent;
    std::memcpy(&stored_significand, value, sizeof(stored_significand));
    std::memcpy(&sign_exponent, static_cast<const unsigned char *>(value) + 8,
                sizeof(sign_exponent));
    result.negative = bool(sign_exponent & 32768);
    const unsigned stored_exponent = sign_exponent & 32767;
    if (stored_exponent == 32767) {
        result.kind = (stored_significand & 9223372036854775807ull) ? 2 : 1;
        result.significand = result.kind == 2 ? stored_significand : 0;
        return result;
    }
    if (!stored_significand)
        return result;
    result.exponent = stored_exponent ? int(stored_exponent) - 16383 : -16382;
    result.significand = stored_significand;
    while (!(result.significand >> 63)) {
        result.significand <<= 1;
        --result.exponent;
    }
    return result;
}
Extended Extended::integer(std::int64_t value, Format mode) {
    Extended result(0.f, mode);
    result.negative = value < 0;
    auto magnitude =
        result.negative ? std::uint64_t(0) - std::uint64_t(value) : std::uint64_t(value);
    if (magnitude) {
        auto leading = top({magnitude, 0});
        result.exponent = int(leading);
        result.significand = magnitude << (63 - leading);
    }
    return result;
}
Extended Extended::invalid(Format f) {
    Extended result(0, f);
    result.kind = 2;
    result.negative = true;
    result.significand = 13835058055282163712ull;
    return result;
}
Extended Extended::propagate(const Extended &a, const Extended &b) {
    Extended result = a.kind == 2 ? a : b;
    if (a.kind == 2 && b.kind == 2 && (a.significand < b.significand))
        result = b;
    result.significand |= 4611686018427387904ull;
    return result;
}
Extended Extended::rounded(Extended::Bits input, int scale, bool sign, Format f, bool sticky) {
    ::nfs::kernel::numeric::Wide value{input.low, input.high};
    Extended result(0, f);
    result.negative = sign;
    if (!nonzero(value))
        return result;
    const unsigned leading = top(value);
    const int shift = int(leading) - int(f.precision) + 1;
    ::nfs::kernel::numeric::Wide retained =
        shift > 0 ? right(value, unsigned(shift)) : left(value, unsigned(-shift));
    bool guard = shift > 0 && bit(value, unsigned(shift - 1));
    bool tail = sticky || (shift > 1 && below(value, unsigned(shift - 1)));
    auto significand = retained.low;
    result.exponent = scale + int(leading);
    if (increment(sign, f.rounding, guard, tail, significand)) {
        ++significand;
        if (!significand || (f.precision < 64 && (significand >> f.precision))) {
            significand = std::uint64_t(1) << (f.precision - 1);
            ++result.exponent;
        }
    }
    result.significand = significand << (64 - f.precision);
    if (result.exponent > 16383) {
        result.kind = 1;
        result.significand = 0;
    }
    return result;
}
Extended Extended::operator-() const {
    Extended result = *this;
    result.negative = !result.negative;
    return result;
}
Extended Extended::square_root() const {
    if (kind == 2)
        return propagate(*this, *this);
    if (!kind && !significand)
        return *this;
    if (negative)
        return invalid(format);
    if (kind == 1)
        return *this;
    // FSQRT value calculation: retain 65 root bits and the exact remainder
    // before applying the saved precision/rounding mode. Status/traps are separate.
    const int parity = exponent & 1;
    const int input_shift = 65 + parity;
    Wide root{}, remainder{};
    auto input_bit = [&](int position) -> unsigned {
        position -= input_shift;
        return position >= 0 && position < 64 ? unsigned((significand >> position) & 1) : 0;
    };
    for (int pair = 64; pair >= 0; --pair) {
        remainder = left(remainder, 2);
        remainder.low |= (input_bit(pair * 2 + 1) << 1) | input_bit(pair * 2);
        auto trial = left(root, 2);
        trial.low |= 1;
        root = left(root, 1);
        if (!less(remainder, trial)) {
            remainder = subtract(remainder, trial);
            root.low |= 1;
        }
    }
    return rounded({root.low, root.high}, (exponent - parity) / 2 - 64, false, format,
                   nonzero(remainder));
}
bool operator<(const Extended &a, const Extended &b) {
    if (a.kind == 2 || b.kind == 2)
        return false;
    const bool az = !a.kind && !a.significand, bz = !b.kind && !b.significand;
    if (az && bz)
        return false;
    if (az)
        return !b.negative;
    if (bz)
        return a.negative;
    if (a.negative != b.negative)
        return a.negative;
    bool less_magnitude;
    if (a.kind || b.kind)
        less_magnitude = a.kind < b.kind;
    else
        less_magnitude =
            a.exponent < b.exponent || (a.exponent == b.exponent && a.significand < b.significand);
    bool equal = a.kind == b.kind &&
                 (a.kind || (a.exponent == b.exponent && a.significand == b.significand));
    return a.negative ? !less_magnitude && !equal : less_magnitude;
}
Extended operator+(const Extended &a, const Extended &b) {
    if (a.kind == 2 || b.kind == 2)
        return Extended::propagate(a, b);
    if (a.kind || b.kind) {
        if (a.kind && b.kind && a.negative != b.negative)
            return Extended::invalid(a.format);
        return a.kind ? a : b;
    }
    if (!a.significand && !b.significand) {
        Extended result = a;
        result.negative = a.negative == b.negative ? a.negative : a.format.rounding == 1;
        return result;
    }
    if (!a.significand)
        return b;
    if (!b.significand)
        return a;
    const int exponent = a.exponent > b.exponent ? a.exponent : b.exponent;
    Wide first = jam(left({a.significand, 0}, 63), unsigned(exponent - a.exponent));
    Wide second = jam(left({b.significand, 0}, 63), unsigned(exponent - b.exponent));
    Wide magnitude;
    bool sign = a.negative;
    if (a.negative == b.negative)
        magnitude = add(first, second);
    else if (less(first, second)) {
        magnitude = subtract(second, first);
        sign = b.negative;
    } else {
        magnitude = subtract(first, second);
        if (!nonzero(magnitude))
            sign = a.format.rounding == 1;
    }
    return Extended::rounded({magnitude.low, magnitude.high}, exponent - 126, sign, a.format);
}
Extended operator*(const Extended &a, const Extended &b) {
    if (a.kind == 2 || b.kind == 2)
        return Extended::propagate(a, b);
    if (a.kind || b.kind) {
        if ((!a.kind && !a.significand) || (!b.kind && !b.significand))
            return Extended::invalid(a.format);
        Extended result = a.kind ? a : b;
        result.negative = a.negative != b.negative;
        return result;
    }
    Wide p = product(a.significand, b.significand);
    return Extended::rounded({p.low, p.high}, a.exponent + b.exponent - 126,
                             a.negative != b.negative, a.format);
}
Extended operator/(const Extended &a, const Extended &b) {
    if (a.kind == 2 || b.kind == 2)
        return Extended::propagate(a, b);
    const bool sign = a.negative != b.negative;
    if ((a.kind && b.kind) || (!a.kind && !a.significand && !b.kind && !b.significand))
        return Extended::invalid(a.format);
    Extended result(0, a.format);
    result.negative = sign;
    if (a.kind || (!b.kind && !b.significand)) {
        result.kind = 1;
        return result;
    }
    if (b.kind || !a.significand)
        return result;
    int exponent = a.exponent - b.exponent;
    Wide remainder{a.significand, 0}, denominator{b.significand, 0}, quotient{};
    if (less(remainder, denominator)) {
        remainder = left(remainder, 1);
        --exponent;
    }
    for (int i = 127; i >= 0; --i) {
        if (!less(remainder, denominator)) {
            remainder = subtract(remainder, denominator);
            quotient = add(quotient, left({1, 0}, unsigned(i)));
        }
        remainder = left(remainder, 1);
    }
    return Extended::rounded({quotient.low, quotient.high}, exponent - 127, sign, a.format,
                             nonzero(remainder));
}
Extended::operator float() const {
    std::uint32_t bits = negative ? 2147483648u : 0;
    if (kind == 1)
        bits |= 2139095040u;
    else if (kind == 2)
        bits |= 2143289344u | std::uint32_t((significand >> 40) & 4194303);
    else if (significand) {
        if (exponent >= -126) {
            auto r = rounded({significand, 0}, exponent - 63, negative, {24, format.rounding});
            if (r.kind || r.exponent > 127) {
                bool infinity = format.rounding == 0 || (format.rounding == 1 && negative) ||
                                (format.rounding == 2 && !negative);
                bits |= infinity ? 2139095040u : 2139095039u;
            } else
                bits |= (std::uint32_t(r.exponent + 127) << 23) |
                        std::uint32_t((r.significand >> 40) & 8388607);
        } else {
            const unsigned shift = unsigned(-86 - exponent);
            Wide value{significand, 0};
            auto retained = right(value, shift).low;
            bool guard = shift && bit(value, shift - 1),
                 sticky = shift > 1 && below(value, shift - 1);
            if (increment(negative, format.rounding, guard, sticky, retained))
                ++retained;
            bits |= std::uint32_t(retained);
        }
    }
    float result;
    std::memcpy(&result, &bits, 4);
    return result;
}
Extended::operator double() const {
    std::uint64_t bits = negative ? 9223372036854775808ull : 0;
    if (kind == 1)
        bits |= 9218868437227405312ull;
    else if (kind == 2)
        bits |= 9221120237041090560ull | ((significand >> 11) & 2251799813685247ull);
    else if (significand) {
        if (exponent >= -1022) {
            auto value = rounded({significand, 0}, exponent - 63, negative, {53, format.rounding});
            if (value.kind || value.exponent > 1023) {
                bool infinity = format.rounding == 0 || (format.rounding == 1 && negative) ||
                                (format.rounding == 2 && !negative);
                bits |= infinity ? 9218868437227405312ull : 9218868437227405311ull;
            } else
                bits |= (std::uint64_t(value.exponent + 1023) << 52) |
                        ((value.significand >> 11) & 4503599627370495ull);
        } else {
            unsigned shift = unsigned(-1011 - exponent);
            Wide value{significand, 0};
            auto retained = right(value, shift).low;
            bool guard = shift && bit(value, shift - 1),
                 sticky = shift > 1 && below(value, shift - 1);
            if (increment(negative, format.rounding, guard, sticky, retained))
                ++retained;
            bits |= retained;
        }
    }
    double result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
std::int64_t Extended::truncate_to_integer() const {
    if (kind || exponent >= 63)
        return INT64_MIN;
    if (!significand || exponent < 0)
        return 0;
    auto magnitude = significand >> unsigned(63 - exponent);
    return negative ? -std::int64_t(magnitude) : std::int64_t(magnitude);
}
std::int64_t Extended::round_to_integer() const {
    if (kind || exponent >= 63)
        return INT64_MIN;
    if (!significand)
        return 0;
    const unsigned shift = unsigned(63 - exponent);
    const Wide value{significand, 0};
    auto magnitude = right(value, shift).low;
    const bool guard = shift && bit(value, shift - 1),
               sticky = shift > 1 && below(value, shift - 1);
    if (increment(negative, format.rounding, guard, sticky, magnitude))
        ++magnitude;
    if (magnitude >= 9223372036854775808ull)
        return INT64_MIN;
    return negative ? -std::int64_t(magnitude) : std::int64_t(magnitude);
}
} // namespace nfs::kernel::numeric
