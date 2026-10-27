#include "nfs/graphics/backend/eagl.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>
namespace nfs::eagl {

static void* __cdecl default_allocate(Dword bytes,const char*){return std::malloc(bytes);}
static void __cdecl default_release(void* memory,Dword){std::free(memory);}
Allocate allocator=default_allocate;Release deallocator=default_release;
ErrorOutput error_output=nullptr;int error_level=10;Dword initial_capacity=256;
SymbolPool external_symbols={0,256,nullptr,0,nullptr};CallbackPools constructors={{0,256,nullptr,0,nullptr},{0,256,nullptr,0,nullptr}},runtime_allocators={{0,256,nullptr,0,nullptr},{0,256,nullptr,0,nullptr}};
// The fallback FILE has descriptor 1.
int report(int level,const char* format,...){int result=0;if(level<=error_level){va_list arguments;va_start(arguments,format);
    result=error_output?error_output(format,arguments):std::vfprintf(stdout,format,arguments);va_end(arguments);}return result;}
// unknown_c is deliberately untouched.
SymbolPool* pool_construct(SymbolPool* p){p->count=0;p->capacity=initial_capacity;p->entries=nullptr;p->resolvers=nullptr;return p;}

Dword pool_hash(SymbolPool* p,const char* name){Dword hash=0;while(*name){hash=hash^(hash<<5)^static_cast<std::int32_t>(*name++);}return hash%p->capacity;}

void pool_insert(SymbolPool* p,const char* name,SymbolEntry* entry){if(!p->entries){p->capacity=initial_capacity;p->entries=static_cast<SymbolEntry**>(allocator(initial_capacity*4,"EAGL::SymbolPool::mpSymbolTable"));
    for(Dword i=0;i<p->capacity;++i)p->entries[i]=nullptr;}
    Dword index=pool_hash(p,name);while(p->entries[index]){++index;if(index>=p->capacity)index=0;}p->entries[index]=entry;++p->count;
    if(p->capacity<=p->count){Dword old_capacity=p->capacity;auto** old=p->entries;p->capacity*=2;
        p->entries=static_cast<SymbolEntry**>(allocator(old_capacity*8,"EAGL::SymbolPool::mpSymbolTable"));for(Dword i=0;i<p->capacity;++i)p->entries[i]=nullptr;
        p->count=0;for(int i=0;i<static_cast<int>(old_capacity);++i)pool_insert(p,old[i]->name,old[i]);if(old)deallocator(old,old_capacity);}}

char* pool_register(SymbolPool* p,const char* name,Dword value){auto* entry=static_cast<SymbolEntry*>(allocator(static_cast<Dword>(std::strlen(name))+5,"EAGL::SymbolEntry"));
    entry->value=value;std::strcpy(entry->name,name);pool_insert(p,name,entry);return entry->name;}
// Fallback list is reached only after a matching table slot re-reads as null; a general
// missing-symbol fallback is not added.
Dword pool_lookup(SymbolPool* p,const char* name,std::uint8_t* found){if(!p->entries){*found=0;return 0;}Dword index=pool_hash(p,name),probes=0;
    for(;;){SymbolEntry* entry=p->entries[index];if(entry&&!std::strcmp(name,entry->name)){entry=p->entries[index];if(entry){*found=1;return entry->value;}
        Dword result=0;*found=0;for(Resolver* r=p->resolvers;r;r=r->next){if(result)return result;result=r->callback(name,found);}return result;}
        if(p->capacity<probes){*found=0;return 0;}if(++index>=p->capacity)index=0;++probes;}}

void pool_remove(SymbolPool* p,const char* name){if(!p->entries)return;Dword index=pool_hash(p,name),probes=0;
    for(;;){SymbolEntry* entry=p->entries[index];if(entry&&!std::strcmp(name,entry->name)){entry=p->entries[index];if(!entry)return;
        deallocator(entry,static_cast<Dword>(std::strlen(entry->name))+5);p->entries[index]=nullptr;--p->count;return;}
        if(p->capacity<probes)return;if(++index>=p->capacity)index=0;++probes;}}

void pool_clear(SymbolPool* p){if(p->entries)for(Dword i=0;i<p->capacity;++i){SymbolEntry* entry=p->entries[i];if(entry)deallocator(entry,static_cast<Dword>(std::strlen(entry->name))+5);p->entries[i]=nullptr;}
    p->count=0;Resolver* current=p->resolvers;while(current){p->resolvers=current->next;deallocator(current,12);current=p->resolvers;}p->resolvers=nullptr;}

void pool_destroy(SymbolPool* p){pool_clear(p);if(p->entries){deallocator(p->entries,p->capacity*4);p->entries=nullptr;}}

void callbacks_construct(CallbackPools* p){pool_construct(&p->construct);pool_construct(&p->destroy);}
void callbacks_clear(CallbackPools* p){pool_clear(&p->construct);pool_clear(&p->destroy);}
void callbacks_destroy(CallbackPools* p){pool_destroy(&p->construct);pool_destroy(&p->destroy);pool_destroy(&p->destroy);pool_destroy(&p->construct);}

void callbacks_register(CallbackPools* p,const char* name,Dword construct,Dword destroy){pool_register(&p->construct,name,construct);pool_register(&p->destroy,name,destroy);}
void callbacks_remove(CallbackPools* p,const char* name){pool_remove(&p->construct,name);pool_remove(&p->destroy,name);}
}
