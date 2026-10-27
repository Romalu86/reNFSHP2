#pragma once
#include "nfs/gui/events/gui_events.hpp"
namespace nfs::gui {
extern Object *engine_object;
void __fastcall reset_engine_queue_counts(Object *);
void __fastcall bind_engine(Object *);
void __fastcall unbind_engine(Object *);
Object *active_engine();
using ModalCallback = void(__cdecl *)(void *);
int run_modal(void *, ModalCallback);
Dword poll_context(void *);
void *focused_widget();
void pump_context(ContextState *);
void tick_registered_timers();
void reset_timer_deadlines(Dword);
Dword post_target(Dword, Dword, Dword, Dword);
namespace events {
void *find_widget(ContextState *, Dword);
int compare_widget_id(const void *, const void *);
} // namespace events
} // namespace nfs::gui
