#include "mad_codec.hpp"
namespace nfs::movies::mad::codec {
namespace {
void butterfly(Dword source, Dword spacing, Dword odd_five, Dword odd_three, Dword odd_one,
               Dword odd_seven) {
    Dword odd_left = odd_three + odd_five, odd_right = odd_seven + odd_one;
    Dword first_difference = odd_five - odd_three, second_difference = odd_one - odd_seven;
    word(7147560, first_difference);
    word(7147564, second_difference);
    Dword odd_sum = odd_left + odd_right;
    Dword odd_rotation = signed_high_product(odd_right - odd_left, word(7147576)) * 2;

    // /arch:IA32 /fp:strict; double retains that precision and the caller's rounding.
    // Signed32 inputs and saved constants keep nonzero values in double's normal range.
    const double first = std::int32_t(first_difference);
    const double second = std::int32_t(second_difference);
    const double second_scaled = second * real(7147584);
    const double sum = second + first;
    const double first_scaled = first * real(7147580);
    const double shared = sum * real(7147588);
    const double first_combined = first_scaled + shared;
    const double second_combined = second_scaled - shared;
    const double first_result = first_combined + real(7147592);
    const double second_result = second_combined + real(7147592);
    Dword even_two = load<Dword>(address(source + 8));
    Dword even_six = load<Dword>(address(source + 24));
    // Native FSTP double followed by DWORD loads: preserve representation, not numeric casts.
    store(transform_storage + 16, second_result);
    store(transform_storage + 8, first_result);
    Dword first_bits = word(7147560), second_bits = word(7147568);
    Dword odd_second = first_bits + odd_rotation;
    Dword odd_third = odd_rotation + second_bits;
    Dword odd_fourth = second_bits + odd_sum;
    word(7147564, odd_second);
    word(7147568, odd_third);
    word(7147572, odd_fourth);
    Dword even_zero = load<Dword>(address(source));
    Dword even_four = load<Dword>(address(source + 16));
    Dword even_sum = even_four + even_zero, even_difference = even_zero - even_four;
    Dword even_rotation = signed_high_product(even_two - even_six, word(7147576)) * 2;
    Dword even_inner = even_six + even_two + even_rotation;
    Dword even_second = even_rotation + even_difference;
    Dword even_third = even_difference - even_rotation;
    Dword even_outer = even_inner + even_sum;
    Dword even_middle = even_sum - even_inner;
    Dword output = word(7147556);
    // The store order is the saved butterfly order2,5,3,4,0,1,6,7.
    store(address(output + 2 * spacing), even_third + odd_second);
    store(address(output + 5 * spacing), even_third - odd_second);
    store(address(output + 3 * spacing), even_middle + first_bits);
    store(address(output + 4 * spacing), even_middle - first_bits);
    store(address(output), even_outer + odd_fourth);
    store(address(output + spacing), odd_third + even_second);
    store(address(output + 6 * spacing), even_second - odd_third);
    store(address(output + 7 * spacing), even_outer - odd_fourth);
}
} // namespace
// Source arrives in ESI in the saved helper; made an explicit C++ argument.
void inverse_columns(Dword source) {
    Dword five = load<Dword>(address(source + 20)), three = load<Dword>(address(source + 12));
    Dword two = load<Dword>(address(source + 8)), six = load<Dword>(address(source + 24));
    Dword four = load<Dword>(address(source + 16)), one = load<Dword>(address(source + 4));
    Dword seven = load<Dword>(address(source + 28));
    if ((two | five | six | three | four | one | seven) == 0) {
        Dword dc = load<Dword>(address(source));
        Dword output = word(7147556);
        for (Dword row = 0; row < 8; ++row)
            store(address(output + row * 36), dc);
        return;
    }
    butterfly(source, 36, five, three, one, seven);
}
// Source arrives in ESI in the saved helper.
void inverse_rows(Dword source) {
    Dword five = load<Dword>(address(source + 20));
    Dword three = load<Dword>(address(source + 12));
    Dword one = load<Dword>(address(source + 4));
    Dword seven = load<Dword>(address(source + 28));
    butterfly(source, 4, five, three, one, seven);
}

void inverse_transform(Dword output, Dword stride) {
    word(7147552, stride * 4);
    for (Dword column = 0; column < 8; ++column) {
        word(7147556, address(transform_storage) + 44 + column * 4);
        inverse_columns(address(transform_storage) + 332 + column * 32);
    }
    word(7147556, output);
    inverse_rows(address(transform_storage) + 44);
    for (Dword row = 1; row < 8; ++row) {
        word(7147556, word(7147556) + word(7147552));
        inverse_rows(address(transform_storage) + 44 + row * 36);
    }
}

void fill_dc(Dword output, Dword stride) {
    for (Dword row = 0; row < 8; ++row) {
        for (Dword column = 0; column < 8; ++column)
            store(address(output + column * 4), word(7147884));
        output += stride * 4;
    }
}
} // namespace nfs::movies::mad::codec
