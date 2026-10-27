#include "network_bandwidth_protocol.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::network::bandwidth {

int __cdecl compare_delays(const void *left, const void *right) {
    const Dword first = *static_cast<const Dword *>(left), second = *static_cast<const Dword *>(right);
    return first == second ? 0 : first < second ? -1 : 1;
}

void sort_delays(int count, Dword *values) {
    std::qsort(values, static_cast<Dword>(count), 4, compare_delays);
}

Extended average_delay(int count, Dword *values, unsigned char trim) {
    if (trim && count > 2) {
        std::qsort(values, static_cast<Dword>(count), 4, compare_delays);
        std::memmove(values, values + 1, static_cast<Dword>(count) * 4u - 8u);
        count -= 2;
    }
    const auto format = kernel::numeric::current_format();
    auto sum = Extended(0.f, format);
    for (int index = 0; index < count; ++index)
        sum += Extended::integer(static_cast<std::int32_t>(values[index]),
                                 format); // FIADD signed DWORD.
    const float mean = float(sum / Extended::integer(count, format));
    if (count > 5) {
        sum = Extended(0.f, format);
        const auto threshold = Extended(mean, format) * .25f;
        int accepted = 0;
        for (int index = 0; index < count; ++index) {
            const auto value =
                Extended::integer(values[index], format); // FILD zero-extended QWORD.
            auto difference = value - mean;
            if (difference < Extended(0.f, format))
                difference = -difference;
            if (difference < threshold) {
                sum += value;
                ++accepted;
            }
        }
        if (accepted > 3)
            return sum / Extended::integer(accepted, format);
    }
    return Extended(mean, format);
}
} // namespace nfs::network::bandwidth
