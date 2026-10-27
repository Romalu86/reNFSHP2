#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::kernel {
using TimerHandler=void(__cdecl*)();
extern TimerHandler timer_handlers[8];
extern volatile Dword timer_frequency,timer_ticks;
extern Dword timer_previous,timer_deadline;
Dword timer_initialize(int); void timer_shutdown();
Dword timer_count(); Dword timer_delta(); void timer_reset();
void timer_set_deadline(Dword); Dword timer_deadline_reached(); void timer_wait_deadline();
void timer_add_handler(TimerHandler);
using TaskCallback=Dword(__cdecl*)(Dword,Dword);
void task_initialize();void task_add(TaskCallback,int,Dword);void task_remove(TaskCallback);

extern Signal clock_notification;extern Dword clock_notification_enabled;
void clock_notify();
}
