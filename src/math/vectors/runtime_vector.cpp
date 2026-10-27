#include "nfs/world/resources/runtime_resources.hpp"
#include "retained_vector.hpp"
#include "retained_vector_providers.hpp"
namespace nfs::game::runtime {
kernel::numeric::Extended retained_dot_product(const float *first, const float *second) {
    const auto format = kernel::numeric::current_format();
    unsigned char value[10];
    hardware::store_dot_product(first, second, value);
    return kernel::numeric::Extended::from_native80(value, format);
}

long double dot_product(const float *first, const float *second) {
    // Compatibility callers still have the MSVC binary64 return boundary.
    return double(retained_dot_product(first, second));
}

long double dot_product_entry(const float *first, const float *second) {
    return dot_product(first, second);
}
kernel::numeric::Extended retained_vector_length(const float *value) {
    const auto format = kernel::numeric::current_format();
    unsigned char length[10];
    hardware::store_vector_length(value, length);
    return kernel::numeric::Extended::from_native80(length, format);
}
// X87 summation order is (x*x + y*y) + z*z.
long double vector_length(const float *value) {
    return double(retained_vector_length(value));
}
} // namespace nfs::game::runtime
