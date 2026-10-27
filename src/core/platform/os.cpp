#include "nfs/core/types/kernel.hpp"
#include "nfs/core/console/console.hpp"
#include <process.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs::kernel {

std::uint8_t lock_init(Lock* l){InitializeCriticalSection(&l->native);return 0;}
void lock_destroy(Lock* l){DeleteCriticalSection(&l->native);l->tag=1836414022;}
void lock_enter(Lock* l){EnterCriticalSection(&l->native);}
void lock_leave(Lock* l){LeaveCriticalSection(&l->native);}
bool signal_init(Signal* s){HANDLE h=CreateEventA(nullptr,FALSE,FALSE,nullptr);if(!h)return false;s->native=h;return true;}
void signal_set(Signal* s){SetEvent(s->native);}
void signal_wait(Signal* s){WaitForSingleObject(s->native,INFINITE);}
void signal_destroy(Signal* s){CloseHandle(s->native);}
static ExitHandler exit_handlers[64]{};
void register_exit(ExitHandler h){for(auto p:exit_handlers)if(p==h)return;for(auto& p:exit_handlers)if(!p){p=h;return;}}

void unregister_exit(ExitHandler handler){for(auto& slot:exit_handlers)if(slot==handler){slot=nullptr;return;}}
void run_exit_handlers(){for(int i=63;i>=0;--i){if(exit_handlers[i])exit_handlers[i]();exit_handlers[i]=nullptr;}}
const char* error_file=nullptr;int error_line=0;
static void __cdecl default_error(const char* format,...){
    char text[512];va_list arguments;va_start(arguments,format);
    if(format)std::vsprintf(text,format,arguments);else text[0]=0;va_end(arguments);
    print::write(2,"ERROR: %s",text);if(error_file)print::write(2,"FILE %s LINE %d\n",error_file,error_line);
    run_exit_handlers();std::exit(0);
}
ErrorHandler error_handler=default_error;
void error(const char* format,...){char text[512];va_list args;va_start(args,format);
    if(format)std::vsprintf(text,format,args);else text[0]=0;va_end(args);error_handler("%s",text);}
static Dword __cdecl check_thread_gate(int){return 0;}
ThreadGate thread_gate=check_thread_gate;
static Dword main_initialized=0;static DWORD main_id=0;static HANDLE main_handle=nullptr;
static void __cdecl thread_shutdown() {
    volatile Dword& initialized = main_initialized;
    if (initialized) {
        HANDLE handle = *static_cast<HANDLE const volatile*>(&main_handle);
        initialized = 0;
        CloseHandle(handle);
        *static_cast<HANDLE volatile*>(&main_handle) = nullptr;
        *static_cast<volatile DWORD*>(&main_id) = 0;
    }
}
void thread_initialize() {
    volatile Dword& initialized = main_initialized;
    if (!initialized) {
        main_id = GetCurrentThreadId();
        const HANDLE target_process = GetCurrentProcess();
        const HANDLE current_thread = GetCurrentThread();
        const HANDLE source_process = GetCurrentProcess();
        DuplicateHandle(source_process, current_thread, target_process,
                        &main_handle, 0, FALSE, DUPLICATE_SAME_ACCESS);
        register_exit(thread_shutdown);
        initialized = 1;
    }
}
struct StartPacket { volatile Dword ready; void(__cdecl*without_argument)(); ThreadEntry entry; Dword argument; };
static unsigned __stdcall thread_thunk(void* pointer){auto* p=static_cast<StartPacket*>(pointer);

    // Capture the request fields before clearing ready; the caller may then release its stack storage.
    Dword initial=p->ready;(void)initial;auto without_argument=p->without_argument;ThreadEntry entry=p->entry;Dword argument;
    std::memcpy(&argument,&p->argument,sizeof(argument));p->ready=0;
    if(without_argument)without_argument();else entry(argument);return 0;}
std::uint8_t thread_set_priority(Thread* thread, int priority) {
    HANDLE handle;
    if (!thread)
        handle = *static_cast<HANDLE const volatile*>(&main_handle);
    else if (thread == reinterpret_cast<Thread*>(~Dword(0)))
        handle = GetCurrentThread();
    else
        handle = static_cast<const volatile Thread*>(thread)->native;
    if (!handle)
        return 0;
    int native_priority = THREAD_PRIORITY_NORMAL;
    switch (priority) {
    case -3: native_priority = -15; break;
    case -2: native_priority = -2; break;
    case -1: native_priority = -1; break;
    case 1: native_priority = 1; break;
    case 2: native_priority = 2; break;
    case 3: native_priority = 15; break;
    default: break;
    }
    SetThreadPriority(handle, native_priority);
    return 1;
}
bool thread_start(Thread* t,ThreadEntry fn,Dword argument,Dword,unsigned stack_size,int priority){
    bool result=false;thread_gate(1);StartPacket packet;packet.without_argument=nullptr;packet.entry=fn;packet.argument=argument;
    packet.ready=static_cast<Dword>(_beginthreadex(nullptr,stack_size,thread_thunk,&packet,CREATE_SUSPENDED,&t->id));
    if(packet.ready){result=true;t->native=reinterpret_cast<HANDLE>(packet.ready);thread_set_priority(t,priority);ResumeThread(t->native);
        while(packet.ready)SleepEx(1,TRUE);}
    thread_gate(0);return result;
}
bool thread_start_no_argument(Thread* t,void(__cdecl* fn)(),Dword,unsigned stack_size,int priority){
    bool result=false;thread_gate(1);StartPacket packet;packet.without_argument=fn;packet.entry=nullptr;
    packet.ready=static_cast<Dword>(_beginthreadex(nullptr,stack_size,thread_thunk,&packet,CREATE_SUSPENDED,&t->id));
    if(packet.ready){result=true;t->native=reinterpret_cast<HANDLE>(packet.ready);thread_set_priority(t,priority);ResumeThread(t->native);
        while(packet.ready)SleepEx(1,TRUE);}
    thread_gate(0);return result;
}
Dword thread_finished(Thread* thread) {
    DWORD code;
    if (!GetExitCodeThread(thread->native, &code))
        return 0;
    return code != STILL_ACTIVE ? 1u : 0u;
}
void thread_close(Thread* t){CloseHandle(t->native);}
bool is_thread(Thread* thread) {
    if (thread == reinterpret_cast<Thread*>(~Dword(0)))
        return true;
    const DWORD current = GetCurrentThreadId();
    const DWORD target = thread ? static_cast<const volatile Thread*>(thread)->id
                                : *static_cast<const volatile DWORD*>(&main_id);
    return current == target;
}
void thread_yield(Dword ms){SleepEx(ms,TRUE);}
Task tasks[16]{};volatile Dword task_ticks=0;
Dword task_ticks_snapshot=0;

Dword cooperate(Dword argument) {
    Dword result = 0;
    task_ticks_snapshot = task_ticks;
    for (auto &task : tasks) {
        const auto callback = task.callback;
        if (!callback)
            continue;
        const Dword deadline = task.next;
        const Dword now = task_ticks;
        if (static_cast<std::int32_t>(now) < static_cast<std::int32_t>(deadline) || task.busy)
            continue;
        const Dword invocation_tick = task_ticks;
        task.busy = 1;
        result |= callback(argument, invocation_tick - deadline);

        const Dword period = static_cast<const volatile Task &>(task).period;
        const Dword next = task_ticks + period;
        task.busy = 0;
        task.next = next;
    }
    return result;
}
}
