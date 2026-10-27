#pragma once
#include "nfs/resources/files/filesystem.hpp"
namespace nfs::files {
namespace cache {
struct Settings {
    Dword size;Allocate allocator;Release deallocator;char* destination;
    std::int64_t byte_limit;char* source;Dword unknown_1c;std::int64_t free_space_limit;
};
struct Entry {char name[260];std::uint8_t cached;};
struct Header {
    Dword size,version,checksum;int count;std::uint64_t bytes;Dword partial_bytes;
    char partial_name[260];std::uint16_t partial_checksum;std::uint8_t reserved[6];
};
struct Input {char name[260];int order,priority;};
struct Queued {Queued* next;Input input;};
static_assert(sizeof(Settings)==40&&offsetof(Settings,source)==24);
static_assert(sizeof(Header)==296&&offsetof(Header,partial_checksum)==288);
static_assert(sizeof(Entry)==261&&sizeof(Input)==268&&sizeof(Queued)==272);
extern Settings settings;extern Header* database;extern Dword enabled;
extern volatile std::uint8_t busy,stopping,pausing,initialized;
extern std::uint8_t dirty;extern kernel::Lock lock;extern kernel::Queue queue;
extern kernel::Signal signal;extern kernel::Thread thread;
std::uint8_t get_settings(Settings*);std::uint8_t set_settings(const Settings*);
void initialize();void shutdown();int state();void pause();void start();void flush();
void enqueue(const Input*,int);int enqueue_list(const char*);
std::uint8_t store(const char*,const void*,Dword,int);std::uint8_t copy(const char*,int);
Dword stream(Dword,HANDLE,int,std::uint16_t*);Entry* insert(const char*);
Dword account_existing(const char*);void create_directories(const char*);void report_error(Dword);
int __cdecl compare_entry(const void*,const void*);
namespace application {
struct Paths {char installation[500];std::uint8_t unknown_1f4[4];char media[500];};
static_assert(offsetof(Paths,media)==504&&sizeof(Paths)==1004);
extern Paths paths;extern std::uint8_t disabled,active,media_available;
Dword discover();Dword initialize();void shutdown();char* directory();
Dword account_existing(const char*);void enqueue(const Input*,int);int enqueue_list(const char*,Dword);
void start();void pause();
}
}
namespace file {
struct BuildInfo {Dword value_0,value_4;const char* build;char payload_0c[80];};
static_assert(sizeof(BuildInfo)==92);
BuildInfo* build_info();
extern int priority;
Dword length(const char*);Dword try_length(const char*);
void* load(const char*,Dword);void* try_load(const char*,Dword);
void* load_with_length(const char*,Dword*,Dword);void* try_load_with_length(const char*,Dword*,Dword);
Dword load_into(const char*,void*,int);Dword try_load_into(const char*,void*,int);
bool load_many(int,const char*const*,void**,Dword);bool try_load_many(int,const char*const*,void**,Dword);
std::uint8_t* load_directory(const char*,Dword);std::uint8_t* try_load_directory(const char*,Dword);
bool save(const char*,const void*,int);bool try_save(const char*,const void*,int);bool exists(const char*);
}
}
