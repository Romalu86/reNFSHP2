#pragma once
#include "nfs/network/session/network_construction.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
namespace nfs::lifecycle::construction::virtual_entries {
using kernel::Dword;
void* __fastcall ai_group_delete(Dword*,void*,unsigned);void* __fastcall ai_events_delete(Dword*,void*,unsigned);void* __fastcall ai_bank_delete(Dword*,void*,unsigned);
void* __fastcall event_delete(Dword*,void*,unsigned);void* __fastcall event_link_delete(Dword*,void*,unsigned);void* __fastcall event_base_delete(Dword*,void*,unsigned);void* __fastcall event_member_delete(Dword*,void*,unsigned);
void* __fastcall skeleton_set_delete(Dword*,void*,unsigned);void* __fastcall skeleton_item_delete(Dword*,void*,unsigned);
}
