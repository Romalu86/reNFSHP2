#include <winsock2.h>
#include "network_bandwidth_protocol.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/core/timing/timer.hpp"
#include <cstring>
namespace nfs::network::bandwidth {
kernel::Signal probe_finished{};
kernel::Thread probe_thread{};
static void set_default_addresses() {
    static constexpr const char *defaults[]{"159.153.224.10", "www.ea.com", "159.153.192.22"};
    for (int index = 0; index < 3; ++index)
        if (!*services::dependency::pending::bandwidth_probe_address(index))
            services::dependency::pending::set_bandwidth_probe_address(index, defaults[index]);
}

void initialize() {
    measured_download = measured_upload = 0;
    historical_rate = read_history();
    probe_phase = 1;
    kernel::signal_init(&probe_finished);
    kernel::thread_start_no_argument(&probe_thread, probe_worker, 0, 0, 0);
    set_default_addresses(); // Order: the worker starts before these writes.
}
static void complete_probe() {
    const Dword deadline = kernel::timer_count() + 4000u;
    while (!kernel::thread_finished(&probe_thread))
        if (!(static_cast<std::int32_t>(deadline) <
              static_cast<std::int32_t>(kernel::timer_count())))
            break;
    kernel::thread_close(&probe_thread);
    kernel::signal_destroy(&probe_finished);
    probe_phase = 2;
    if (measured_download)
        save_history(measured_download);
}

void stop() {
    if (probe_phase == 1)
        complete_probe();
}

void poll() {
    if (probe_phase == 1 && kernel::signal_ready(&probe_finished) && probe_phase == 1)
        complete_probe();
}

void retest() {
    if (probe_phase != 2)
        return;
    measured_download = measured_upload = 0;
    historical_rate = read_history();
    probe_phase = 1;
    kernel::signal_init(&probe_finished);
    kernel::thread_start_no_argument(&probe_thread, probe_worker, 0, 0, 0);
    set_default_addresses();
}

void probe_worker() {
    probe_host(services::dependency::pending::bandwidth_probe_address(0), &measured_download,
               &measured_upload);
    if (!measured_download)
        probe_host(services::dependency::pending::bandwidth_probe_address(1), &measured_download,
                   &measured_upload);
    if (!measured_download)
        probe_host(services::dependency::pending::bandwidth_probe_address(2), &measured_download,
                   &measured_upload);
    kernel::signal_set(&probe_finished);
}

void probe_host(const char *host, Dword *download_rate, Dword *upload_rate) {
    *download_rate = *upload_rate = 0;
    WSADATA sockets;
    if (WSAStartup(257, &sockets))
        return;
    hostent *entry = gethostbyname(host);
    WSACleanup();
    if (!entry)
        return;
    Dword address;
    std::memcpy(&address, entry->h_addr_list[0], entry->h_length);
    const Dword remote = ntohl(address);
    Dword options = 1644744, upstream = 0;
    int status = 0;
    Dword downstream = measure_rate(remote, 0, services::dependency::bandwidth_port, 2, &status,
                                   &upstream, 257, &options);
    if (!downstream && (status == 7 || status == 8)) {
        options = (options & 4294905959u) | 4196u;
        status = 0;
        upstream = 0;
        downstream = measure_rate(remote, 0, services::dependency::bandwidth_port, 0, &status,
                                  &upstream, 257, &options);
    }
    *download_rate = quantize_rate(downstream >> 10);
    *upload_rate = quantize_rate(upstream >> 10);
}
} // namespace nfs::network::bandwidth
