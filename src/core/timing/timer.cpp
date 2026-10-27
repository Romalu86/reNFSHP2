#include "nfs/core/timing/timer.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <mmsystem.h>
#include <cstring>
namespace nfs::kernel {
TimerHandler timer_handlers[8]{};
volatile Dword timer_frequency=0,timer_ticks=0;
Dword timer_previous=0,timer_deadline=0;
static Dword interval_fixed=0,fraction=0,next_event=0,resolution=0;
static Signal timer_signal{};static Thread timer_thread{};
static volatile Dword enabled=0,event_id=0,worker_running=0,paused=0;
static Dword late_events=0,initialized=0,task_mutation=0;
Signal clock_notification{};Dword clock_notification_enabled=0;

void clock_notify(){if(clock_notification_enabled)signal_set(&clock_notification);}
// WinMM's actual stdcall callback ABI (RET 14h).
static void CALLBACK timer_callback(UINT,UINT,DWORD_PTR,DWORD_PTR,DWORD_PTR){
    const Dword interval = static_cast<const volatile Dword &>(interval_fixed);
    const Dword carried_fraction = static_cast<const volatile Dword &>(fraction);
    const Dword accumulated = interval + carried_fraction;
    const Dword previous_deadline = static_cast<const volatile Dword &>(next_event);
    const Dword deadline = previous_deadline + (static_cast<std::int32_t>(accumulated) >> 16);

    const Dword timer_enabled = enabled;
    static_cast<volatile Dword &>(next_event) = deadline;
    static_cast<volatile Dword &>(fraction) = accumulated & 65535;
    if (timer_enabled) {
        const Dword now = GetTickCount();
        Dword delay = next_event - now - 1;
        if (static_cast<std::int32_t>(delay) < 1) {
            if (static_cast<std::int32_t>(delay) < -1000) {
                static_cast<volatile Dword &>(next_event) = GetTickCount() + 1;
                ++static_cast<volatile Dword &>(late_events);
            }
            delay = 1;
        }
        const Dword scheduled_event = timeSetEvent(delay, resolution, timer_callback, 0, 0);
        event_id = scheduled_event;

        if (!scheduled_event) {
            timer_frequency = 0;
            enabled = 0;
            timeEndPeriod(static_cast<const volatile Dword &>(resolution));
        }
    }
    if (worker_running && !paused)
        signal_set(&timer_signal);
    clock_notify();
}

static void __cdecl timer_worker(){signal_init(&timer_signal);worker_running=1;signal_wait(&timer_signal);
    while(enabled){++timer_ticks;++task_ticks;GetTickCount();
        for(unsigned offset=0;offset<8;++offset)if(timer_handlers[offset])timer_handlers[offset]();
        signal_wait(&timer_signal);}
    signal_destroy(&timer_signal);worker_running=0;timer_frequency=0;
}
// This function does not close the timer thread handle.
void timer_shutdown(){if(enabled){const Dword pending_event=event_id;timer_frequency=0;enabled=0;
    if(pending_event)timeKillEvent(pending_event);timeEndPeriod(resolution);
    if(worker_running)signal_set(&timer_signal);
    while(event_id&&worker_running)thread_yield(1);}
    fraction=0;paused=0;resolution=0;event_id=0;
}

Dword timer_initialize(int frequency){int attempts=5;if(!initialized){initialized=1;std::memset(timer_handlers,0,sizeof(timer_handlers));}
    for(;;){timer_shutdown();if(!frequency)frequency=100;
        else if(frequency<1||frequency>10000){error("TIMER_init - BAD TIMER FREQUENCY SPECIFIED %d\n",frequency);frequency=100;}
        interval_fixed=static_cast<Dword>(65536000/frequency);fraction=0;timer_ticks=0;
        TIMECAPS caps;MMRESULT result=timeGetDevCaps(&caps,8);
        if(result){error("TIMER_init - MULTIMEDIA TIMER NOT FOUND.\n");timer_frequency=0;enabled=0;return 0;}
        caps.wPeriodMin=1;caps.wPeriodMax=1;resolution=1;
        if(static_cast<std::int32_t>(interval_fixed)<65536){error("TIMER_init - MULTIMEDIA TIMER CANNOT SUPPORT THIS FREQUENCY.\n");timer_frequency=0;enabled=0;return 0;}
        memory_zero(&timer_thread,12);
        if(!thread_start_no_argument(&timer_thread,timer_worker,0,0,2)){
            error("TIMER_init - FAILED TO CREATE TIMER THREAD.\n");timer_frequency=0;enabled=0;return 0;}
        result=timeBeginPeriod(resolution);
        if(result){timer_frequency=0;enabled=0;signal_set(&timer_signal);
            error("TIMER_init - FAILED TO INITIALIZE MULTIMEDIA TIMER : 0x%x.\n",result);return timer_frequency;}
        register_exit(timer_shutdown);timer_frequency=frequency;enabled=frequency;
        if(frequency&&!event_id){const Dword previous_pause=paused;paused=1;next_event=GetTickCount();timer_callback(0,0,0,0,0);paused=previous_pause;}
        Dword start_ticks=timer_count(),start=GetTickCount(),span=timer_frequency*5;
        while(static_cast<std::int32_t>(GetTickCount())<static_cast<std::int32_t>(start+span)&&timer_count()==start_ticks)thread_yield(1);
        if(timer_count()==start_ticks)timer_shutdown();if(timer_frequency)return timer_frequency;
        if(!--attempts){error("TIMER_init - FAILED TO INITIALIZE WINDOWS MULTIMEDIA TIMER, TRY RE-RUNNING.\n");return timer_frequency;}
    }
}

Dword timer_count(){return timer_ticks;}
// (ECX preload must precede the call).
Dword timer_delta(){Dword previous=timer_previous;timer_previous=timer_count();return timer_previous-previous;}

void timer_reset(){timer_ticks=0;Dword now=timer_ticks;timer_previous=now;timer_deadline=now;}

void timer_set_deadline(Dword ticks){timer_deadline=timer_count()+ticks;}

Dword timer_deadline_reached(){return static_cast<std::int32_t>(timer_count()-timer_deadline)>=0?1u:0u;}

void timer_wait_deadline(){while(!timer_deadline_reached())cooperate(0);}

void timer_add_handler(TimerHandler handler){for(auto current:timer_handlers)if(current==handler)return;
    for(auto& current:timer_handlers)if(!current){current=handler;return;}}

void task_initialize(){memory_fill(tasks,0,256);}
// Preserve the mutation-counter-dependent slot choice.
void task_add(TaskCallback callback,int period,Dword delay){if(period==-1)period=0;else if(!period)period=1;
    Dword skipped_empty_slots=task_mutation;++task_mutation;int selected=-1;
    for(int i=0;i<16;++i){int candidate=i;
        if(tasks[i].callback!=callback){candidate=selected;if(!tasks[i].callback&&selected==-1){candidate=i;if(skipped_empty_slots){--skipped_empty_slots;candidate=selected;}}}
        selected=candidate;}
    if(selected==-1){error("SYNCTASK_add - LIST FULL\n");--task_mutation;return;}
    tasks[selected]={callback,static_cast<Dword>(period),task_ticks+delay,0};--task_mutation;
}

void task_remove(TaskCallback callback){for(int i=0;i<16;++i)if(tasks[i].callback==callback){
    if(i<16&&tasks[i].callback==callback)tasks[i].callback=nullptr;return;}}
}
