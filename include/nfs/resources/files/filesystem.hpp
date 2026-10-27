#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::files {
using kernel::Dword;
using Callback=void(__cdecl*)(Dword,int,Dword);
using Allocate=void*(__cdecl*)(const char*,int,Dword);
using Release=std::uint8_t(__cdecl*)(void*);
extern Allocate& allocate;extern Release& release;
void set_memory(Allocate,Release);
Dword initialize(int handles,int unused,int operations);void shutdown();
int storage_size(int handles,int unused,int operations);Dword initialize_in(void*,int handles,int unused,int operations);
Dword open(const char*,Dword,int,Dword=0);Dword close(Dword,int,Dword=0);
Dword read(Dword,int,void*,int,int,Dword=0);Dword write(Dword,int,const void*,int,int,Dword=0);
Dword size(Dword,int,Dword=0);Dword exists(const char*,int,Dword=0);Dword erase(const char*,int,Dword=0);
Dword truncate(Dword,int,int,Dword=0);Dword mount(const char*,Dword,int,Dword=0);Dword unmount(Dword,int,Dword=0);
int poll(Dword);int wait(Dword);Dword complete(Dword);void cancel(Dword);void set_callback(Dword,Callback);
Dword context(Dword);Dword operation_device(Dword);void set_priority(Dword,std::uint8_t);
using Atomic=Dword(__cdecl*)(int,Dword);
Dword atomic(Atomic,int,int,Dword);
bool open_sync(const char*,Dword,int,Dword*);Dword read_sync(Dword,int,void*,int,int);Dword write_sync(Dword,int,const void*,int,int);
bool close_sync(Dword,int);Dword size_sync(Dword,int);bool exists_sync(const char*,int);bool erase_sync(const char*,int);
bool truncate_sync(Dword,int,int);bool mount_sync(const char*,Dword,int,Dword*);bool unmount_sync(Dword,int);
int path_device(const char*);void set_directory(const char*);void get_directory(char*);

namespace detail {
struct Request { Request* next;Dword token,operation,flags;std::int8_t status;std::uint8_t priority;std::uint16_t unknown_12;
    Dword win32_error,file,argument;Callback callback;Dword value_24,value_28,value_2c; };
struct Device {volatile int active;kernel::Thread thread;Request* current;kernel::Queue pending,completed;
    kernel::Signal wake;kernel::Lock atomic_lock;HANDLE completion;Dword serial;volatile int atomic_priority;};
struct Mount {Mount* next;Dword token,device,file;std::uint8_t* data;char name[260];};
static_assert(sizeof(Request)==48 && sizeof(Device)==172 && sizeof(Mount)==280);
static_assert(offsetof(Request,callback)==32 && offsetof(Device,atomic_lock)==132);
extern Device* devices;extern kernel::Queue free_requests,mounts;extern volatile int stop;
int compare_token(void*,Dword);Request* find(Device*,Dword,int*);Request* reserve(Dword,std::uint8_t,Dword,Dword);
Dword enqueue(Request*);void start_device(int);void copy_path(Request*,const char*);Dword open_backend_or_mount(const char*,Dword,bool);
}
namespace backend {
struct Handle {std::uint8_t used;std::int8_t device;volatile std::int8_t reading;std::uint8_t unknown_3;
    HANDLE file;Dword flags,chunk;HANDLE mapping;std::uint8_t* data;std::int32_t position,length;};
static_assert(sizeof(Handle)==32);
extern Handle* handles;extern int handle_count;
void initialize(int,void*);void shutdown();Dword open(const char*,Dword,Dword*);int reserve();
Dword information(Dword,Dword*,Dword*);int read(Dword,void*,int);int write(Dword,const void*,int);
Dword seek(Dword,int);Dword close(Dword);Dword erase(const char*);Dword memory_file(void*,Dword,std::uint8_t,Dword*);
void* data(Dword);Dword truncate(Dword,int);int cancel_read(Dword);int device(Dword);bool is_memory(Dword);
int path_drive(const char*);std::uint8_t hint(const char*); // original default C-locale comparator
void path_root(const char*,char*);bool full_path(const char*,char*,Dword);int search_drive(const char*,char*);
std::uint64_t free_space(const char*);
}
namespace async {
using Completion=void(__cdecl*)(Dword);
struct Request {Dword token;Request* next;Dword transferred;int released,aborted;Dword owned_buffer;Completion callback;
    volatile Dword operation;Dword file,offset;int remaining;std::uint8_t* cursor;};
static_assert(sizeof(Request)==48);
void initialize(int,Dword);void shutdown();Dword load(const char*,Dword);Dword load_into(const char*,void*,int);
Dword read(Dword,Dword,void*,int);void set_callback(Dword,Completion);int poll(Dword);int cancel(Dword);
int take(Dword,void**,Dword*);
}
}
