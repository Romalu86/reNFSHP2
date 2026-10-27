#include "nfs/network/session/network_virtual.hpp"
#include "nfs/core/lifecycle/system_destruction.hpp"
#include "nfs/core/lifecycle/system_delete_entries.hpp"
namespace nfs::lifecycle::construction::network::entries {
static Dword* before(Dword* object,Dword bytes){return reinterpret_cast<Dword*>(reinterpret_cast<std::uintptr_t>(object)-bytes);}
static Dword* virtual_before(Dword* object,Dword field,Dword bytes){Dword displacement=*reinterpret_cast<const volatile Dword*>(reinterpret_cast<std::uintptr_t>(object)-field);return before(object,displacement+bytes);}

void* __fastcall interface_first_delete(Dword* object,void*,unsigned flags){object[0]=reinterpret_cast<Dword>(pending::comparer_interface_methods);if(flags&1)gui::application_release(object);return object;}

void* __fastcall interface_second_delete(Dword* object,void*,unsigned flags){object[0]=reinterpret_cast<Dword>(pending::visitor_interface_methods);if(flags&1)gui::application_release(object);return object;}

void voting_destroy(Dword* object){Dword* second=object?object+18:nullptr;*second=reinterpret_cast<Dword>(pending::visitor_interface_methods);
    Dword* first=object?object+17:nullptr;*first=reinterpret_cast<Dword>(pending::comparer_interface_methods);destruction::thread(object);}

void* __fastcall thread_delete(Dword* object,void*,unsigned flags){destruction::thread(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall worker_delete(Dword* object,void*,unsigned flags){destruction::network_game_base(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall server_worker_delete(Dword* object,void*,unsigned flags){destruction::network_server_base(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall voting_delete(Dword* object,void*,unsigned flags){voting_destroy(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall game_worker_delete(Dword* object,void*,unsigned flags){return table_entries::network_game_delete_entry(reinterpret_cast<System*>(before(object,96)),nullptr,flags);}

void* __fastcall game_first_delete(Dword* object,void*,unsigned flags){return table_entries::network_game_delete_entry(reinterpret_cast<System*>(before(object,164)),nullptr,flags);}

void* __fastcall game_second_delete(Dword* object,void*,unsigned flags){return table_entries::network_game_delete_entry(reinterpret_cast<System*>(before(object,168)),nullptr,flags);}

void* __fastcall worker_first_delete(Dword* object,void*,unsigned flags){return worker_delete(before(object,68),nullptr,flags);}

void* __fastcall worker_second_delete(Dword* object,void*,unsigned flags){return worker_delete(before(object,72),nullptr,flags);}

void* __fastcall server_worker_adjust_delete(Dword* object,void*,unsigned flags){return table_entries::network_server_delete_entry(reinterpret_cast<System*>(virtual_before(object,4,320)),nullptr,flags);}

void* __fastcall server_first_delete(Dword* object,void*,unsigned flags){return table_entries::network_server_delete_entry(reinterpret_cast<System*>(virtual_before(object,72,388)),nullptr,flags);}

void* __fastcall server_second_delete(Dword* object,void*,unsigned flags){return table_entries::network_server_delete_entry(reinterpret_cast<System*>(virtual_before(object,4,396)),nullptr,flags);}

void* __fastcall voting_first_delete(Dword* object,void*,unsigned flags){return voting_delete(before(object,68),nullptr,flags);}

void* __fastcall voting_second_delete(Dword* object,void*,unsigned flags){return voting_delete(before(object,72),nullptr,flags);}

void* __fastcall server_interface_delete(Dword* object,void*,unsigned flags){return server_worker_delete(before(object,68),nullptr,flags);}

Dword __fastcall server_dispatch_adjust(Dword* object,void*,Dword first,Dword second,Dword third,Dword fourth){return server_dispatch(virtual_before(object,4,0),nullptr,first,second,third,fourth);}

Dword __fastcall server_second_dispatch(Dword* object,void*,Dword first,Dword second,Dword third,Dword fourth){return server_second_dispatch_adjust(virtual_before(object,4,0),nullptr,first,second,third,fourth);}
}
