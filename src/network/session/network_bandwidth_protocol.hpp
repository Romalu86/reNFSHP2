#pragma once
#include <winsock2.h>
#include "network_bandwidth.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::network::bandwidth {
using kernel::numeric::Extended;
extern SOCKET datagram_socket, echo_socket;
extern Dword probes_sent, probes_timed_out, delays_in_band, delays_received;
extern unsigned char measurement_succeeded;
extern Dword probe_sequence;
extern char address_text[128], endpoint_text[128];
Dword measure_rate(Dword remote, Dword local, std::uint16_t port, int retries, int *status,
                  Dword *upstream, Dword version, const Dword *options);
Dword measure_route(Dword remote, Dword local, std::uint16_t port, int *status, Dword *upstream,
                   const Dword *options);
int sample_delays(Dword remote, Dword local, int count, int payload_size, Dword *delays,
                  Dword timeout);
int __cdecl compare_delays(const void *, const void *);
void sort_delays(int count, Dword *);
Extended average_delay(int count, Dword *, unsigned char trim);
bool send_echo(const void *, int payload_size, SOCKET, const sockaddr *,
               Dword sequence);
bool receive_echo(SOCKET, int *sequence, sockaddr *, Dword expected_destination,
                  Dword *received_destination);
const char *format_probe_address(const Dword *);
} // namespace nfs::network::bandwidth
