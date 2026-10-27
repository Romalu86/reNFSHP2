#pragma once
#include "nfs/core/lifecycle/system_construction.hpp"
namespace nfs::lifecycle::construction::network {
Dword* thread_construct(Dword*,const Dword*);void thread_configure(Dword*,const Dword*);Dword* server_thread(Dword*);Dword* voting_booth(Dword*);
extern const std::int32_t game_bases[2],server_bases[3];
namespace pending {
extern const Dword thread_methods[],comparer_interface_methods[],visitor_interface_methods[];
extern const Dword game_worker_methods[],game_worker_first[],game_worker_second[];
extern const Dword game_worker_final[],game_first_final[],game_second_final[];
extern const Dword server_worker_methods[],server_worker_interface[];
extern const Dword server_worker_final[],server_first_final[],server_second_final[];
extern const Dword voting_methods[],voting_first[],voting_second[];
}
}
