#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::gui {
using kernel::Dword;

struct Host { void(__cdecl*unused)(); void*(__cdecl*create_lock)();
    void(__cdecl*enter)(void*);void(__cdecl*leave)(void*);void(__cdecl*destroy_lock)(void*); };
struct Settings {Dword flags,pool;int bytes;Dword alignment,alternate_alignment;};
static_assert(sizeof(Host)==20&&sizeof(Settings)==20);
extern Host* host;extern Settings settings;extern Dword pool;extern int pool_bytes;
extern void* backing;extern Dword initialized,application_pool;
extern void* allocation_break_address;extern Dword& allocation_break_enabled;extern Dword host_lock_count;
Dword initialize(Host*,int,const Settings*,const char*);Dword shutdown();
extern Host application_host;
void __cdecl host_unused();void* __cdecl create_lock();void __cdecl enter(void*);void __cdecl leave(void*);void __cdecl delete_lock(void*);
void* application_allocate(int);void application_release(void*);
void* allocate(int);void* allocate_flags(int,Dword,const char*);void release(void*);
void release_flags(void*,Dword,const char*);
void* reallocate(void*,const char*,int,Dword);
// Object/data layouts below cover the lifecycle/INI call paths. They do not
// claim to implement the other engine virtual interfaces used by rendering.
using Destroy=void*(__thiscall*)(void*,unsigned);
struct Object {const Destroy* methods;};
extern const Destroy base_methods[],container_methods[],string_methods[];
void destroy(Object*);
using Dispose=void(__cdecl*)(void*);
struct Element {const Destroy* methods;void* value;Dispose dispose;};
struct Container {const Destroy* methods;Element** elements;Dword count,flags;};
struct List {const Destroy* methods;Object* first;Object* last;void* lock;Dword count,granularity;};
struct String {const Destroy* methods;void* data;Dword reference;int length;Dword wide;};
struct PoolString {String string;int references;};
static_assert(sizeof(Object)==4&&sizeof(Element)==12&&sizeof(Container)==16&&sizeof(List)==24&&sizeof(String)==20&&sizeof(PoolString)==24);
extern Container* configuration_documents;
extern Object* string_resolver;
extern Container* pooled_strings;
extern Dword container_mode;
void container_construct(Container*);void container_destruct(Container*);Dword container_clear(Container*);
Dword container_append(Container*,void*,Dispose);Dword container_insert(Container*,void*,Dword,Dispose);
Dword container_remove(Container*,void*);
Dword container_detach(Container *, void *);
Dword container_set_at(Container*,void*,Dword,Dispose);
Dword container_move(Container*,Dword,Dword);
Dword container_remove_at(Container*,Dword);
using Compare=int(__cdecl*)(const void*,const void*);
void* container_find(Container*,Compare,const void*);Dword container_sorted_insert(Container*,Compare,const void*,void*,Dispose);
Dword container_find_index(Container*,Compare,const void*);
void dispose_object(void*);
void clear_registry_objects();void reset_registry_time();
const void* string_data(const String*);const char* string_ascii(const String*);
const void* resolve_string(Dword);int resolve_length(Dword);Dword resolve_wide(Dword);
String* string_reference(String*,Dword);String* string_construct_reference(String*,Dword);
String* string_construct_copy(String*,const String*);String* string_construct_ascii(String*,const char*);
bool string_make_narrow(String*);bool string_make_wide(String*);
String* string_assign_wide(String*,const wchar_t*);void string_append_reference(String*,Dword);
String* copy_member_string(const void*,String*);
void string_construct(String*);String* string_assign(String*,const char*);void string_clear(String*);void string_destruct(String*);
void string_copy(String*,const String*);void string_append(String*,const String*);bool string_equal_folded(const String*,const char*);
void string_append_ascii(String*,const char*);
int string_compare_ascii(const String*,const char*);
int string_compare_inline_ascii(const String*,const char*);
PoolString* string_acquire(const char*);void string_release(PoolString*);
}
