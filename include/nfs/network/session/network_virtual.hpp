#pragma once
#include "nfs/core/lifecycle/nested_interfaces.hpp"
namespace nfs::lifecycle::construction::network::entries {
using kernel::Dword;
void* allocate_payload(int);void release_payload(void*);
Dword __fastcall clone_payload(Dword*,void*,Dword,Dword);void __fastcall destroy_payload(Dword*,void*,Dword);Dword __fastcall payload_size(Dword*,void*,Dword);void __fastcall copy_payload(Dword*,void*,Dword,Dword,Dword);
Dword __fastcall default_compare(Dword*,void*,Dword,Dword);Dword __fastcall default_dispatch(Dword*,void*,Dword,Dword,Dword,Dword);
Dword __fastcall default_pre_dispatch(Dword*,void*,Dword,Dword,Dword);void __fastcall default_post_dispatch(Dword*,void*,Dword,Dword);
Dword __fastcall compare_word(Dword*,void*,Dword,Dword);Dword __fastcall compare_second_word(Dword*,void*,Dword,Dword);Dword __fastcall compare_server_word(Dword*,void*,Dword,Dword);Dword __fastcall compare_vote_byte(Dword*,void*,Dword,Dword);
Dword __fastcall vote_count_first(Dword*,void*,Dword,Dword,Dword,Dword);Dword __fastcall vote_count_second(Dword*,void*,Dword,Dword,Dword,Dword);Dword __fastcall compare_vote(Dword*,void*,Dword,Dword);
Dword __fastcall filter_message(Dword*,void*,Dword,Dword,Dword,Dword);Dword __fastcall forward_message(Dword*,void*,Dword,Dword,Dword,Dword);
void voting_destroy(Dword*);void* __fastcall interface_first_delete(Dword*,void*,unsigned);void* __fastcall interface_second_delete(Dword*,void*,unsigned);
void* __fastcall thread_delete(Dword*,void*,unsigned);void* __fastcall worker_delete(Dword*,void*,unsigned);void* __fastcall server_worker_delete(Dword*,void*,unsigned);void* __fastcall voting_delete(Dword*,void*,unsigned);
void* __fastcall game_worker_delete(Dword*,void*,unsigned);void* __fastcall game_first_delete(Dword*,void*,unsigned);void* __fastcall game_second_delete(Dword*,void*,unsigned);
void* __fastcall worker_first_delete(Dword*,void*,unsigned);void* __fastcall worker_second_delete(Dword*,void*,unsigned);
void* __fastcall server_worker_adjust_delete(Dword*,void*,unsigned);void* __fastcall server_first_delete(Dword*,void*,unsigned);void* __fastcall server_second_delete(Dword*,void*,unsigned);
void* __fastcall voting_first_delete(Dword*,void*,unsigned);void* __fastcall voting_second_delete(Dword*,void*,unsigned);void* __fastcall server_interface_delete(Dword*,void*,unsigned);
Dword __fastcall server_dispatch(Dword*,void*,Dword,Dword,Dword,Dword);Dword __fastcall server_dispatch_adjust(Dword*,void*,Dword,Dword,Dword,Dword);Dword __fastcall server_second_dispatch_adjust(Dword*,void*,Dword,Dword,Dword,Dword);
Dword __fastcall server_compare_adjust(Dword*,void*,Dword,Dword);
Dword __fastcall server_second_dispatch(Dword*,void*,Dword,Dword,Dword,Dword);
}
