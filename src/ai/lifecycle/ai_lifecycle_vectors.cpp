#include "nfs/math/vectors/vector_arrays.hpp"

namespace nfs::game::runtime {

const float *add_vector_and_advance(const float *first, const float *second, float *output) {
    const long double x = static_cast<long double>(first[0]) + second[0];
    const long double y = static_cast<long double>(first[1]) + second[1];
    const long double z = static_cast<long double>(first[2]) + second[2];
    output[0] = static_cast<float>(x);
    output[1] = static_cast<float>(y);
    output[2] = static_cast<float>(z);
    return first + 3;
}

const float *subtract_vector_and_advance(const float *first, const float *second, float *output) {
    const long double x = static_cast<long double>(first[0]) - second[0];
    const long double y = static_cast<long double>(first[1]) - second[1];
    const long double z = static_cast<long double>(first[2]) - second[2];
    output[0] = static_cast<float>(x);
    output[1] = static_cast<float>(y);
    output[2] = static_cast<float>(z);
    return first + 3;
}

void add_constant_vector_array(int count, const float *input, const float *constant,
                               float *output) {
    while (count > 0) {
        input = add_vector_and_advance(input, constant, output);
        output += 3;
        --count;
    }
}

void subtract_constant_vector_array(int count, const float *input, const float *constant,
                                    float *output) {
    while (count > 0) {
        input = subtract_vector_and_advance(input, constant, output);
        output += 3;
        --count;
    }
}
} // namespace nfs::game::runtime
