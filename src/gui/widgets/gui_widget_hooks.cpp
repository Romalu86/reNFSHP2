#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/core/lifecycle/array_construction.hpp"
#include <cstdlib>
#include "gui_widget_hooks.hpp"
#include "../lifecycle/gui_configuration_cache.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::gui {
extern const Destroy scoped_container_methods[];
}
namespace nfs::gui::widgets {
using game::runtime::invoke;
NameRecord *names{};
EventRecord *named_events{};
CallbackRecord *callbacks{};
static EventRecord builtin_events[92]{};

NameRecord *copy_name_record(NameRecord *record, const NameRecord *source) {
    record->name = source->name;
    record->hash = source->hash;
    record->next = names;
    names = record;
    return record;
}

NameRecord *register_name(NameRecord *record, const char *name) {
    record->name = name;
    engine::pending::hash_name(&record->hash, name);
    record->next = names;
    names = record;
    return record;
}

Dword find_name_hash(const char *name) {
    NameRecord *record = names;
    Dword hash;
    engine::pending::hash_name(&hash, name);
    if (hash)
        for (; record; record = record->next)
            if (record->hash == hash)
                return record->hash;
    return 0;
}

const char *find_registered_name(Dword hash) {
    if (hash)
        for (NameRecord *record = names; record; record = record->next)
            if (record->hash == hash)
                return record->name;
    return nullptr;
}

void destroy_name_record(void *object) {
    auto *record = static_cast<NameRecord *>(object);
    NameRecord *previous = nullptr;
    for (NameRecord *current = names; current; current = current->next) {
        if (current == record) {
            if (previous)
                previous->next = record->next;
            else
                names = record->next;
            return;
        }
        previous = current;
    }
}


EventRecord *copy_event_record(EventRecord *record, const EventRecord *source) {
    record->event = source->event;
    record->hash = source->hash;
    record->name = source->name;
    record->next = named_events;
    named_events = record;
    return record;
}

EventRecord *register_event(EventRecord *record, Dword event, const char *name) {
    record->event = event;
    record->name = name;
    engine::pending::hash_name(&record->hash, name);
    record->next = named_events;
    named_events = record;
    return record;
}

Dword find_registered_event(const char *name) {
    Dword hash;
    engine::pending::hash_name(&hash, name);
    if (hash)
        for (EventRecord *record = named_events; record; record = record->next)
            if (record->hash == hash)
                return record->event;
    return 0;
}

const char *event_name(Dword event) {
    if (event)
        for (EventRecord *record = named_events; record; record = record->next)
            if (record->event == event)
                return record->name;
    return nullptr;
}

static void destroy_event_record(EventRecord *record) {
    EventRecord *previous = nullptr;
    for (EventRecord *current = named_events; current; current = current->next) {
        if (current == record) {
            if (previous)
                previous->next = record->next;
            else
                named_events = record->next;
            return;
        }
        previous = current;
    }
}

static void destroy_named_events() {
    lifecycle::construction::objects::destroy_array(builtin_events, sizeof(EventRecord), 92,
                                                       destroy_event_record);
}

EventRecord *link_event(EventRecord *record) {
    record->next = named_events;
    named_events = record;
    return record;
}

void initialize_named_events() {
    static const struct {
        const char *name;
        Dword event;
    } definitions[] = {
#include "../events/gui_named_events.inc"
    };
    static_assert(sizeof(definitions) / sizeof(definitions[0]) == 92);
    for (unsigned index = 0; index < 92; ++index) {
        EventRecord *record = builtin_events + index;
        if (index < 42) {
            record->event = definitions[index].event;
            record->name = definitions[index].name;
            engine::pending::hash_name(&record->hash, record->name);
            link_event(record);
        } else
            register_event(record, definitions[index].event, definitions[index].name);
    }
}
// Registration follows the completed population helper.
static void initialize_named_event_owner() {
    initialize_named_events();
    std::atexit(destroy_named_events);
}
NFS_KERNEL_INITIALIZER(startup_initialize_named_event_owner, ".CRT$XCU_NFS_006906E8",
                       initialize_named_event_owner);

CallbackRecord *register_callback(CallbackRecord *record, TypeName type, const char *event_name,
                                  Dword event, const char *callback_name, Callback callback,
                                  Dword flags) {
    record->event = event;
    record->type = type;
    engine::pending::hash_name(&record->event_hash, event_name);
    record->event_name = event_name;
    record->callback_name = callback_name;
    engine::pending::hash_name(&record->callback_hash, callback_name);
    record->callback = callback;
    record->flags = flags;
    record->next = callbacks;
    callbacks = record;
    return record;
}

void unregister_callback(CallbackRecord *record) {
    CallbackRecord *previous = nullptr;
    for (auto *current = callbacks; current; current = current->next) {
        if (current == record) {
            if (previous)
                previous->next = record->next;
            else
                callbacks = record->next;
            return;
        }
        previous = current;
    }
}

Dword hook_command(Container *container, Dword event) {
    const Dword count = container->count;
    for (Dword index = 0; index < count; ++index)
        if (auto *hook = static_cast<Hook *>(events::container_value(container, index)))
            if (hook->command && hook->event == event)
                return hook->command;
    return 0;
}

Callback hook_callback(Container *container, Dword event) {
    const Dword count = container->count;
    for (Dword index = 0; index < count; ++index)
        if (auto *hook = static_cast<Hook *>(events::container_value(container, index)))
            if (hook->callback && hook->event == event)
                return hook->callback;
    return nullptr;
}

// it captures the count before disposal and retains the hole if compaction fails.
static Dword remove_hook(Container *container, Hook *hook) {
    return container_remove(container, hook);
}
extern const Destroy hook_methods[];

Dword set_hook_command(Container *container, Dword event, Dword command) {
    if (event == 35)
        return 4;
    const Dword count = container->count;
    for (Dword index = 0; index < count; ++index)
        if (auto *hook = static_cast<Hook *>(events::container_value(container, index)))
            if (hook->event == event && hook->command) {
                if (command) {
                    hook->command = command;
                    return 0;
                }
                return remove_hook(container, hook);
            }
    auto *hook = static_cast<Hook *>(allocate_flags(sizeof(Hook), 0, "GHook"));
    if (!hook)
        return 1;
    hook->methods = hook_methods;
    hook->command = command;
    hook->event = event;
    hook->callback = nullptr;
    Dword result = container_append(container, hook, dispose_object);
    if (result)
        invoke(hook, 0, Dword(1));
    return result;
}

Dword set_hook_callback(Container *container, Dword event, Callback callback) {
    const Dword count = container->count;
    for (Dword index = 0; index < count; ++index)
        if (auto *hook = static_cast<Hook *>(events::container_value(container, index)))
            if (hook->event == event && hook->callback) {
                if (callback) {
                    hook->callback = callback;
                    return 0;
                }
                return remove_hook(container, hook);
            }
    auto *hook = static_cast<Hook *>(allocate_flags(sizeof(Hook), 0, "GHook"));
    if (!hook)
        return 1;
    hook->methods = hook_methods;
    hook->callback = callback;
    hook->event = event;
    hook->command = 0;
    Dword result = container_append(container, hook, dispose_object);
    if (result)
        invoke(hook, 0, Dword(1));
    return result;
}
Container *create_hook_container() {
    auto *container =
        static_cast<Container *>(allocate_flags(sizeof(Container), 0, "GHookContainer"));
    if (container) {
        container_construct(container);
        container->methods = scoped_container_methods;
    }
    return container;
}

static void destroy_hook(Hook *hook) {
    hook->methods = gui::base_methods;
}

static Hook *__fastcall delete_hook(Hook *hook, void *, Dword flags) {
    destroy_hook(hook);
    if (flags & 1)
        release(hook);
    return hook;
}
const Destroy hook_methods[] = {reinterpret_cast<Destroy>(delete_hook)};
} // namespace nfs::gui::widgets
