#pragma once
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/input/directinput/directinput.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
namespace nfs::services::dependency {
using kernel::Dword;
using Byte = std::uint8_t;
struct FilterBucket {
    char name[500];
    Dword accepted, rejected, bytes, recent;
};
static_assert(sizeof(FilterBucket) == 516);
extern FilterBucket message_filters[53], combined_filter, udp_filter, tcp_filter;
const char *message_name(int);
void set_filter_name(FilterBucket *, const char *);
void reset_filter(FilterBucket *);
void initialize_network_filters();
void reset_network_filters();
extern Byte preference_flags[5], frontend_diagnostics;
extern char (&preferred_player_name)[80];
extern char recent_addresses[5][16];
extern int maximum_concurrent_updates;
extern std::uint16_t internet_query_port, lan_query_port, lan_query_base, bandwidth_port,
    local_tcp_port;
void load_network_settings();
void release_network_settings();
void load_server_settings(void *);
void set_maximum_concurrent_updates(int);
void set_recent_address(int, const char *);
const char *recent_address(int);
const char *preferred_name();
Dword set_ini_text(ini::Reader *, const char *, const char *, const char *);
void set_internet_query_port(std::uint16_t);
void set_lan_query_port(std::uint16_t);
void set_local_tcp_port(std::uint16_t);
void set_local_udp_port(std::uint16_t);
Byte enable_feedback(Dword);
void set_feedback_gain(Dword, Dword);
Dword create_feedback_effect(Dword, Dword *);
int find_unused_effect();
void download_effect(int);
void unload_effect(int);
void download_effect_with_retry(IDirectInputDevice2A *, IDirectInputEffect *);
namespace pending {
extern const GUID effect_guids[13];
void set_bandwidth_probe_address(int,
                                 const char *);
const char *bandwidth_probe_address(int);
} // namespace pending
} // namespace nfs::services::dependency
