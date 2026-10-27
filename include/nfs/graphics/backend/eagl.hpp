#pragma once
#include "nfs/core/types/kernel.hpp"
#include <cstdarg>
namespace nfs::eagl {
using kernel::Dword;
using Allocate=void*(__cdecl*)(Dword,const char*);using Release=void(__cdecl*)(void*,Dword);
extern Allocate allocator;extern Release deallocator;
using ErrorOutput=int(__cdecl*)(const char*,va_list);extern ErrorOutput error_output;extern int error_level;
int report(int,const char*,...);
using Resolve=Dword(__cdecl*)(const char*,std::uint8_t*);
struct Resolver {Resolve callback;Dword unknown_4;Resolver* next;};
struct SymbolEntry {Dword value;char name[1];};
struct SymbolPool {Dword count,capacity;SymbolEntry** entries;Dword unknown_c;Resolver* resolvers;};
struct CallbackPools {SymbolPool construct,destroy;};
extern Dword initial_capacity;extern SymbolPool external_symbols;extern CallbackPools constructors,runtime_allocators;
static_assert(sizeof(SymbolPool)==20&&sizeof(CallbackPools)==40&&sizeof(Resolver)==12);
SymbolPool* pool_construct(SymbolPool*);void pool_clear(SymbolPool*);void pool_destroy(SymbolPool*);
Dword pool_hash(SymbolPool*,const char*);void pool_insert(SymbolPool*,const char*,SymbolEntry*);
char* pool_register(SymbolPool*,const char*,Dword);void pool_remove(SymbolPool*,const char*);Dword pool_lookup(SymbolPool*,const char*,std::uint8_t*);
void callbacks_construct(CallbackPools*);void callbacks_clear(CallbackPools*);void callbacks_destroy(CallbackPools*);
void callbacks_register(CallbackPools*,const char*,Dword,Dword);void callbacks_remove(CallbackPools*,const char*);
#pragma pack(push,1)
struct ElfHeader {std::uint8_t ident[16];std::uint16_t type,machine;Dword version,entry,phoff,shoff,flags;std::uint16_t ehsize,phentsize,phnum,shentsize,shnum,shstrndx;};
struct Section {Dword name,type,flags,address,offset,size,link,info,alignment,entry_size;};
struct Symbol {Dword name,value,size;std::uint8_t info,binding;std::uint16_t section;};
struct Relocation {Dword offset,info;};
#pragma pack(pop)
struct Record {Record* next;Record* previous;char* strings;std::uint8_t resolved;std::uint8_t unknown_d[3];int count;Symbol* symbols;Section* sections;ElfHeader* header;int buckets[256];int* links;Resolve resolve;};
using Destructor=void(__cdecl*)(Dword);
struct Destruction {Destructor callback;Dword value;};
using RuntimeDestroy=void(__cdecl*)(Dword,Dword);
struct RuntimeDestruction {RuntimeDestroy callback;Dword value,context;RuntimeDestruction* next;};
struct Loader {Record* record;int destructor_count;Destruction* destructors;RuntimeDestruction* runtime_destructors;ElfHeader* header;Dword split;std::uint8_t* second;};
struct SymbolView {const char* name;const char* type;Dword address;std::uint8_t defined;std::uint8_t unknown_d[3];};
static_assert(sizeof(ElfHeader)==52&&sizeof(Section)==40&&sizeof(Symbol)==16&&sizeof(Record)==1064&&offsetof(Record,links)==1056&&sizeof(Loader)==28&&sizeof(SymbolView)==16);
extern Record* records;
Dword dynamic_hash(const char*);void prepare(Loader*,Resolve);void resolve(Loader*);void construct_objects(Loader*);
void destroy_objects(Loader*);void detach(Loader*);Loader* loader_construct(Loader*,ElfHeader*,Dword,Resolve);void loader_destroy(Loader*);
int symbol_count(Loader*);void symbol_at(Loader*,SymbolView*,int);Dword record_lookup(Record*,const char*);
std::uint8_t address(Loader*,const char*,const char*,Dword*);std::uint8_t next_symbol(Loader*,const char*,int*,SymbolView*);
Dword next_address(Loader*,const char*,int*,Dword*);
}
