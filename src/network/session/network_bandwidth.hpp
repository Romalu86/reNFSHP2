#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::network::bandwidth {
using kernel::Dword;
extern Dword measured_download, measured_upload, historical_rate;
extern volatile Dword probe_phase;
Dword download();
Dword upload();
Dword limit();
void initialize();
void stop();
void poll();
void retest();
void probe_worker();
void probe_host(const char *, Dword *, Dword *);
Dword quantize_rate(Dword);
void save_history(Dword);
Dword read_history();
} // namespace nfs::network::bandwidth
