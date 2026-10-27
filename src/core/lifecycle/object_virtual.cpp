#include "nfs/core/lifecycle/nested_interfaces.hpp"
#include "nfs/graphics/backend/eagl.hpp"
namespace nfs::lifecycle::construction::virtual_entries {

void* __fastcall ai_group_delete(Dword* object,void*,unsigned flags){objects::ai_group_destroy(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall ai_events_delete(Dword* object,void*,unsigned flags){objects::ai_events_destroy(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall ai_bank_delete(Dword* object,void*,unsigned flags){objects::ai_bank_destroy(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall event_delete(Dword* object,void*,unsigned flags){objects::event_item_destroy(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall event_link_delete(Dword* object,void*,unsigned flags){return event_delete(reinterpret_cast<Dword*>(reinterpret_cast<std::uint8_t*>(object)-28),nullptr,flags);}

void* __fastcall event_base_delete(Dword* object,void*,unsigned flags){object[0]=reinterpret_cast<Dword>(objects::pending::input_event_base_methods);if(flags&1)gui::application_release(object);return object;}

void* __fastcall event_member_delete(Dword* object,void*,unsigned flags){objects::event_member_destroy(object);if(flags&1)gui::application_release(object);return object;}

void* __fastcall skeleton_set_delete(Dword* object,void*,unsigned flags){objects::skeleton_set_destroy(object);if(flags&1)gui::application_release(object);return object;}
// This entry releases through EAGL's sized callback.
void* __fastcall skeleton_item_delete(Dword* object,void*,unsigned flags){objects::skeleton_item_destroy(object);if(flags&1)eagl::deallocator(object,96);return object;}
}
namespace nfs::lifecycle::construction::objects::pending {
using namespace virtual_entries;
const Dword ai_group_methods[]={reinterpret_cast<Dword>(ai_group_delete)};
const Dword ai_events_methods[]={reinterpret_cast<Dword>(ai_events_delete)};
const Dword ai_bank_methods[]={reinterpret_cast<Dword>(ai_bank_delete)};
const Dword event_methods[]={reinterpret_cast<Dword>(event_delete)};
const Dword event_link_methods[]={reinterpret_cast<Dword>(event_link_delete)};
const Dword input_event_base_methods[]={reinterpret_cast<Dword>(event_base_delete)};
const Dword event_member_methods[]={reinterpret_cast<Dword>(event_member_delete)};
const Dword skeleton_set_methods[]={reinterpret_cast<Dword>(skeleton_set_delete)};
const Dword skeleton_item_methods[]={reinterpret_cast<Dword>(skeleton_item_delete)};
}
