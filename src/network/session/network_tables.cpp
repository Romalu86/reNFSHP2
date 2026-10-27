#include "nfs/core/errors/assertions.hpp"
#include "nfs/network/session/network_virtual.hpp"
namespace nfs::lifecycle::construction::network::pending {

const Dword comparer_interface_methods[]={reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),reinterpret_cast<Dword>(entries::interface_first_delete)};

const Dword visitor_interface_methods[]={reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),reinterpret_cast<Dword>(entries::interface_second_delete)};

const Dword game_second_final[]={reinterpret_cast<Dword>(entries::filter_message),reinterpret_cast<Dword>(entries::game_second_delete)};

const Dword game_first_final[]={reinterpret_cast<Dword>(entries::compare_second_word),reinterpret_cast<Dword>(entries::game_first_delete)};

const Dword game_worker_final[]={reinterpret_cast<Dword>(entries::game_worker_delete),reinterpret_cast<Dword>(entries::default_pre_dispatch),reinterpret_cast<Dword>(entries::compare_word),reinterpret_cast<Dword>(entries::forward_message),reinterpret_cast<Dword>(entries::payload_size),reinterpret_cast<Dword>(entries::default_post_dispatch),reinterpret_cast<Dword>(entries::clone_payload),reinterpret_cast<Dword>(entries::destroy_payload),reinterpret_cast<Dword>(entries::copy_payload)};

const Dword game_worker_second[]={reinterpret_cast<Dword>(entries::filter_message),reinterpret_cast<Dword>(entries::worker_second_delete)};

const Dword game_worker_first[]={reinterpret_cast<Dword>(entries::compare_second_word),reinterpret_cast<Dword>(entries::worker_first_delete)};

const Dword game_worker_methods[]={reinterpret_cast<Dword>(entries::worker_delete),reinterpret_cast<Dword>(entries::default_pre_dispatch),reinterpret_cast<Dword>(entries::compare_word),reinterpret_cast<Dword>(entries::forward_message),reinterpret_cast<Dword>(entries::payload_size),reinterpret_cast<Dword>(entries::default_post_dispatch),reinterpret_cast<Dword>(entries::clone_payload),reinterpret_cast<Dword>(entries::destroy_payload),reinterpret_cast<Dword>(entries::copy_payload)};

const Dword server_second_final[]={reinterpret_cast<Dword>(entries::server_second_dispatch),reinterpret_cast<Dword>(entries::server_second_delete)};

const Dword server_first_final[]={reinterpret_cast<Dword>(entries::server_compare_adjust),reinterpret_cast<Dword>(entries::server_first_delete)};

const Dword server_worker_final[]={reinterpret_cast<Dword>(entries::server_worker_adjust_delete),reinterpret_cast<Dword>(entries::default_pre_dispatch),reinterpret_cast<Dword>(entries::compare_server_word),reinterpret_cast<Dword>(entries::server_dispatch_adjust),reinterpret_cast<Dword>(entries::payload_size),reinterpret_cast<Dword>(entries::default_post_dispatch),reinterpret_cast<Dword>(entries::clone_payload),reinterpret_cast<Dword>(entries::destroy_payload),reinterpret_cast<Dword>(entries::copy_payload)};

const Dword voting_second[]={reinterpret_cast<Dword>(entries::vote_count_second),reinterpret_cast<Dword>(entries::voting_second_delete)};

const Dword voting_first[]={reinterpret_cast<Dword>(entries::compare_vote),reinterpret_cast<Dword>(entries::voting_first_delete)};

const Dword voting_methods[]={reinterpret_cast<Dword>(entries::voting_delete),reinterpret_cast<Dword>(entries::default_pre_dispatch),reinterpret_cast<Dword>(entries::compare_vote_byte),reinterpret_cast<Dword>(entries::vote_count_first),reinterpret_cast<Dword>(entries::payload_size),reinterpret_cast<Dword>(entries::default_post_dispatch),reinterpret_cast<Dword>(entries::clone_payload),reinterpret_cast<Dword>(entries::destroy_payload),reinterpret_cast<Dword>(entries::copy_payload)};

const Dword thread_methods[]={reinterpret_cast<Dword>(entries::thread_delete),reinterpret_cast<Dword>(entries::default_pre_dispatch),reinterpret_cast<Dword>(entries::default_compare),reinterpret_cast<Dword>(entries::default_dispatch),reinterpret_cast<Dword>(entries::payload_size),reinterpret_cast<Dword>(entries::default_post_dispatch),reinterpret_cast<Dword>(entries::clone_payload),reinterpret_cast<Dword>(entries::destroy_payload),reinterpret_cast<Dword>(entries::copy_payload)};

const Dword server_worker_interface[]={reinterpret_cast<Dword>(entries::server_compare_adjust),reinterpret_cast<Dword>(entries::server_interface_delete)};

const Dword server_worker_methods[]={reinterpret_cast<Dword>(entries::server_worker_delete),reinterpret_cast<Dword>(entries::default_pre_dispatch),reinterpret_cast<Dword>(entries::compare_server_word),reinterpret_cast<Dword>(entries::default_dispatch),reinterpret_cast<Dword>(entries::payload_size),reinterpret_cast<Dword>(entries::default_post_dispatch),reinterpret_cast<Dword>(entries::clone_payload),reinterpret_cast<Dword>(entries::destroy_payload),reinterpret_cast<Dword>(entries::copy_payload)};
}
