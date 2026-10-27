#include "network_bandwidth.hpp"
namespace nfs::network::bandwidth {
Dword measured_download{}, measured_upload{}, historical_rate{};
volatile Dword probe_phase{};

Dword download() {
    return probe_phase == 2 ? measured_download : 0;
}

Dword upload() {
    return probe_phase == 2 ? measured_upload : 0;
}

Dword limit() {
    const Dword measured = probe_phase == 2 ? measured_download : 0;
    Dword value = historical_rate;
    if (!measured || !value) {
        if (value < measured)
            value = measured;
    } else if (measured < value)
        return measured;
    return value;
}
} // namespace nfs::network::bandwidth
