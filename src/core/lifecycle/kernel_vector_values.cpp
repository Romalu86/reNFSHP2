#include "kernel_values.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::game::runtime::values {

static void *__fastcall delete_vector(VectorValue *value, void *, Dword flags) {
    destroy_vector(value);
    if (flags & 1)
        gui::release(value);
    return value;
}

const gui::Destroy vector_methods[] = {reinterpret_cast<gui::Destroy>(delete_vector)};

void destroy_vector(VectorValue *value) {
    value->methods = gui::base_methods;
}
// All three sums precede the first destination store.
VectorValue *sum_vector_value(VectorValue *output, const VectorValue *first, const float *second) {
    const auto z = static_cast<long double>(first->z) + second[2];
    const auto y = static_cast<long double>(first->y) + second[1];
    const auto x = static_cast<long double>(first->x) + second[0];
    auto &methods = *reinterpret_cast<const gui::Destroy *volatile *>(&output->methods);
    methods = gui::base_methods;
    output->x = float(x);
    methods = vector_methods;
    output->y = float(y);
    output->z = float(z);
    return output;
}

VectorValue *assign_vector_value(VectorValue *output, const float *source) {
    output->x = source[0];
    const float y = source[1], z = source[2];
    output->y = y;
    output->z = z;
    return output;
}
// Retain the sequential loads/stores for overlapping triples.
float *difference_triplets(float *output, const float *first, const float *second) {
    for (unsigned axis = 0; axis < 3; ++axis)
        output[axis] = float(static_cast<long double>(first[axis]) - second[axis]);
    return output;
}
} // namespace nfs::game::runtime::values
