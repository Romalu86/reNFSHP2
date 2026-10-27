#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <cstddef>
namespace nfs::kernel {
static_assert(sizeof(void*)==4,"Original FILESYS runtime requires the x86 ABI");
using Dword=std::uint32_t;
static_assert(sizeof(Dword)==4, "A game DWORD must remain 32 bits");
static_assert(sizeof(std::uint8_t)==1 && sizeof(std::uint16_t)==2,
              "Game byte and 16-bit fields must retain their original widths");
struct Lock { Dword tag; CRITICAL_SECTION native; };
static_assert(sizeof(Lock)==28);
std::uint8_t lock_init(Lock*); void lock_destroy(Lock*); void lock_enter(Lock*); void lock_leave(Lock*);
struct Signal { Dword tag; HANDLE native; };
bool signal_init(Signal*); void signal_set(Signal*); void signal_wait(Signal*); void signal_destroy(Signal*);
Dword signal_ready(Signal*);
struct Thread { Dword tag; HANDLE native; unsigned id; };
using ThreadEntry=void(__cdecl*)(Dword);
using ThreadGate=Dword(__cdecl*)(int);
extern ThreadGate thread_gate;
bool thread_start(Thread*,ThreadEntry,Dword,Dword unused,unsigned stack_size,int priority);
bool thread_start_no_argument(Thread*,void(__cdecl*)(),Dword unused,unsigned,int);
Dword thread_finished(Thread*);void thread_close(Thread*);
void thread_initialize(); bool is_thread(Thread*); void thread_yield(Dword); std::uint8_t thread_set_priority(Thread*,int);
using ExitHandler=void(__cdecl*)();
void register_exit(ExitHandler);void unregister_exit(ExitHandler); void run_exit_handlers();
using ErrorHandler=void(__cdecl*)(const char*,...);
extern ErrorHandler error_handler;
extern const char* error_file;extern int error_line;
void error(const char*,...);
struct Task { Dword(__cdecl*callback)(Dword,Dword); Dword period,next; int busy; };
extern Task tasks[16]; extern volatile Dword task_ticks;extern Dword task_ticks_snapshot;
Dword cooperate(Dword);
extern Dword debug_break_enabled;void debug_break();
using BinaryCompare=int(__cdecl*)(const void*,const void*);
void* binary_search(const void*,const void*,Dword,int,BinaryCompare);
Dword checksum16(const std::uint8_t*,int,std::uint16_t);Dword checksum32(const std::uint8_t*,int);

struct QueueNode { QueueNode* next; };
using QueueKey=Dword(__cdecl*)(const void*,Dword);
using QueueMatch=int(__cdecl*)(void*,Dword);
struct Queue {
    int count; Dword flags; QueueNode* first; QueueNode* last; QueueKey key; Dword argument; Lock lock;
};
static_assert(sizeof(Queue)==52);
void queue_init(Queue*,QueueKey,Dword); void queue_clone_lock(Queue*,QueueKey,Dword,const Queue*);
void queue_destroy(Queue*); void queue_push_front(Queue*,void*); void queue_push_back(Queue*,void*);
void* queue_pop(Queue*); void queue_insert(Queue*,void*);
Dword queue_remove_unlocked(Queue*,void*); Dword queue_remove(Queue*,void*);
void* queue_find(Queue*,QueueMatch,Dword); void* queue_extract(Queue*,QueueMatch,Dword);
void* queue_find_key(Queue*,Dword); void* queue_extract_key(Queue*,Dword);
int queue_visit(Queue*,QueueMatch,Dword); int queue_visit_locked(Queue*,QueueMatch,Dword);

struct Block {
    std::uint16_t magic,flags; std::int32_t size; Block* next; Block* previous;
};
struct FreeBlock { Block block; Block* next_free; Block* previous_free; };
struct Pool {
    char name[8]; Block* first; Block* last; FreeBlock anchor; Dword alignment,alternate_alignment,overhead,flags;
    std::uint8_t locked; std::uint8_t unknown_39[3]; Lock lock; void* owned;
};
static_assert(sizeof(Block)==16 && sizeof(Pool)==92);
static_assert(offsetof(Pool,first)==8 && offsetof(Pool,anchor)==16 && offsetof(Pool,lock)==60 && offsetof(Pool,owned)==88);
extern Pool* pools[64]; extern Dword default_pool; extern int memory_error;
using OutOfMemory=int(__cdecl*)(Dword,int,Dword);
extern OutOfMemory out_of_memory;
int pool_create(Dword,const char*,void*,int,Dword,Dword,Dword,bool,bool,bool);
bool pool_destroy(Dword); int pool_unused(); Dword memory_initialize(unsigned); void memory_shutdown();
void* memory_allocate_aligned(const char*,int,Dword,Dword,Dword);
void* memory_allocate(const char*,int,Dword); std::uint8_t memory_free(void*);
int memory_size(const void*); Dword memory_flags(const void*); void* memory_resize(void*,int);
int memory_largest_free(Dword);
std::uint8_t memory_validate_block(Block*);std::uint8_t memory_validate_free_node(Block*);
std::uint8_t memory_validate_links(Block*);std::uint8_t memory_validate_free_list(Block*);
std::uint8_t memory_validate_pool(Dword);void memory_validate_all();
}
