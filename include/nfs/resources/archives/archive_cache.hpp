#pragma once
#include "nfs/resources/files/filesystem.hpp"
namespace nfs::files::archives {
struct Entry {Dword file;std::uint8_t* directory;char name[260];};
static_assert(sizeof(Entry)==268&&offsetof(Entry,name)==8);
extern Entry entries[9];extern int count,next;extern CRITICAL_SECTION lock;
void* allocate_entry(int,const char*,Dword);
void initialize();void clear();void shutdown();
// acquire is the original unlocked helper; callers hold the cache lock.
Entry* acquire(const char*,Dword);
void* load(const char* archive,const char* entry,Dword flags,std::uint8_t keep_path);
Dword offset(const char* archive,const char* entry,Dword flags);
Dword load_many(const char* archive,const char*const* names,void** output,int,Dword);
}
