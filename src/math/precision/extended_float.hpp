#pragma once
#include <cstdint>
namespace nfs::kernel::numeric {
// C++ representation of the x87 significand/exponent used by the saved formulas.
// It avoids MSVC's 53-bit long double storage. CPU status/trap ABI is separate.
struct Format {
    unsigned precision, rounding;
};
Format current_format();
class Extended {
  public:
    Extended(float, Format);
    static Extended from_double(double, Format);
    // Import the ten meaningful bytes of a canonical FSTP80 result, without rounding.
    static Extended from_native80(const void *, Format);
    static Extended integer(std::int64_t, Format);
    explicit operator float() const;
    explicit operator double() const;
    std::int64_t truncate_to_integer() const;
    std::int64_t round_to_integer() const;
    Extended square_root() const;
    Extended operator-() const;
    Extended &operator+=(const Extended &b) {
        return *this = *this + b;
    }
    friend Extended operator+(const Extended &, const Extended &);
    friend Extended operator-(const Extended &a, const Extended &b) {
        return a + -b;
    }
    friend Extended operator*(const Extended &, const Extended &);
    friend Extended operator/(const Extended &, const Extended &);
    friend bool operator<(const Extended &, const Extended &);
    friend Extended operator+(const Extended &a, float b) {
        return a + Extended(b, a.format);
    }
    friend Extended operator-(const Extended &a, float b) {
        return a - Extended(b, a.format);
    }
    friend Extended operator*(const Extended &a, float b) {
        return a * Extended(b, a.format);
    }
    Extended &operator+=(float b) {
        return *this = *this + b;
    }

  private:
    std::uint64_t significand{};
    int exponent{};
    bool negative{};
    unsigned kind{}; // finite0, infinity1, NaN2.
    Format format;
    struct Bits {
        std::uint64_t low, high;
    };
    static Extended rounded(Bits, int, bool, Format, bool = false);
    static Extended invalid(Format);
    static Extended propagate(const Extended &, const Extended &);
};
} // namespace nfs::kernel::numeric
