#pragma once
#include "gui_widgets.hpp"
namespace nfs::gui::widgets {
using Callback = Dword(__cdecl *)(void *, Dword, Dword, Dword);
using TypeName = const char *(__cdecl *)();
struct NameRecord {
    const char *name;
    Dword hash;
    NameRecord *next;
};
struct EventRecord {
    const char *name;
    Dword event, hash;
    EventRecord *next;
};
struct CallbackRecord {
    TypeName type;
    Callback callback;
    Dword callback_hash;
    const char *callback_name;
    Dword event, event_hash;
    const char *event_name;
    Dword flags;
    CallbackRecord *next;
};
struct Hook {
    const Destroy *methods;
    Dword event;
    Callback callback;
    Dword command;
};
static_assert(sizeof(NameRecord) == 12 && sizeof(EventRecord) == 16);
static_assert(sizeof(CallbackRecord) == 36 && sizeof(Hook) == 16);
extern NameRecord *names;
extern EventRecord *named_events;
extern CallbackRecord *callbacks;
NameRecord *register_name(NameRecord *, const char *);
void destroy_name_record(void *);
CallbackRecord *register_callback(CallbackRecord *, TypeName, const char *, Dword, const char *,
                                  Callback, Dword);
void unregister_callback(CallbackRecord *);
const char *find_registered_name(Dword);
Dword find_registered_event(const char *);
Dword hook_command(Container *, Dword);
Callback hook_callback(Container *, Dword);
Dword set_hook_command(Container *, Dword, Dword);
Dword set_hook_callback(Container *, Dword, Callback);
Container *create_hook_container();
} // namespace nfs::gui::widgets
