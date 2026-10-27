#include "nfs/gui/lifecycle/gui.hpp"
#include "gui_registry_owners.hpp"
#include "nfs/gui/lifecycle/gui_context.hpp"
#include "nfs/gui/lifecycle/gui_modal.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/core/timing/timer.hpp"
#include "../../math/precision/extended_float.hpp"
#include "gui_timer_x87.hpp"
#include <cstring>
namespace nfs::gui {
Host *host = nullptr;
Settings settings{};
Dword pool = 0;
int pool_bytes = 0;
void *backing = nullptr;
Dword initialized = 0;
Dword application_pool = 0, host_lock_count = 0;
Dword &allocation_break_enabled = kernel::debug_break_enabled;
void *allocation_break_address = nullptr;
static void *context_lock = nullptr;
Container *pooled_strings = nullptr;
static_assert(sizeof(ContextState) == 112 && offsetof(ContextState, index) == 40 &&
              offsetof(ContextState, lock) == 84 && offsetof(ContextState, extra) == 92);
ContextState *context_state = nullptr;
Object *engine_object = nullptr;
extern String *clipboard_text;
struct Registry {
    const Destroy *methods;
    Dword references;
    Container objects;
};
static_assert(sizeof(Registry) == 24);
static Registry *registry = nullptr;
Dword registry_time{};

void register_input_object(void *object) {
    Registry *current = registry;
    if (current && object) {
        using Initialize=void(__thiscall *)(void *);
        (*reinterpret_cast<const Initialize **>(object))[2](object);
        container_append(&current->objects,object,dispose_object);
    }
}

void reset_registry_time(){registry_time=kernel::timer_count();}

void clear_registry_objects(){
    Registry *current = registry;
    if(!current)return;
    const Dword count=current->objects.count;
    for(Dword index=0;index<count;++index){
        if(index>=current->objects.count)continue;
        Element *element=current->objects.elements[index];void *value=element?element->value:nullptr;
        if(value){using Reset=void(__thiscall *)(void *);(*reinterpret_cast<const Reset **>(value))[2](value);}
    }
    reset_registry_time();
}

Dword poll_context(void *root) {
    if (void *focused = focused_widget())
        root = focused;
    Registry *current = registry;
    if (!root)
        return 6;
    if (current) {
        const Dword count = current->objects.count;
        for (Dword index = 0; index < count; ++index)
            if (void *object = events::container_value(&current->objects, index)) {
                using Tick = void(__thiscall *)(void *, void *);
                (*reinterpret_cast<const Tick **>(object))[1](object, root);
            }
    }
    return 0;
}
Container timer_entries{container_methods, nullptr, 0, 0};

int compare_timer_id(const void *timer, const void *key) {
    const Dword id = reinterpret_cast<const Dword *>(timer)[3];
    const Dword wanted = reinterpret_cast<Dword>(key);
    return id < wanted ? -1 : wanted < id;
}

void reset_timer_deadlines(Dword id) {
    if (!id) {
        const Dword count = timer_entries.count;
        Element **entries = timer_entries.elements;
        for (Dword index = 0; index < count; ++index)
            if (Element *entry = entries[index])
                if (void *timer = entry->value)
                    reinterpret_cast<Dword *>(timer)[4] = 0;
    } else if (void *timer = container_find(&timer_entries, compare_timer_id, reinterpret_cast<void *>(id))) {
        reinterpret_cast<Dword *>(timer)[4] = 0;
    }
}
Container *configuration_documents = nullptr;
Object *string_resolver = nullptr;
Dword container_mode = 1;
void *text_scratch = nullptr;
Dword text_scratch_busy{};

void *application_allocate(int bytes) {
    void *result = kernel::memory_allocate("eagl new", bytes, application_pool);
    if (allocation_break_address && result == allocation_break_address)
        kernel::debug_break();
    return result;
}
void application_release(void *memory) {
    if (memory)
        kernel::memory_free(memory);
}

void __cdecl host_unused() {
}
void *__cdecl create_lock() {
    auto *lock = static_cast<CRITICAL_SECTION *>(application_allocate(24));
    InitializeCriticalSection(lock);
    ++host_lock_count;
    return lock;
}
void __cdecl enter(void *lock) {
    if (lock)
        EnterCriticalSection(static_cast<CRITICAL_SECTION *>(lock));
}
void __cdecl leave(void *lock) {
    if (lock)
        LeaveCriticalSection(static_cast<CRITICAL_SECTION *>(lock));
}
void __cdecl delete_lock(void *lock) {
    if (lock) {
        DeleteCriticalSection(static_cast<CRITICAL_SECTION *>(lock));
        --host_lock_count;
        application_release(lock);
    }
}
Host application_host = {host_unused, create_lock, enter, leave, delete_lock};

void *allocate(int bytes) {
    return kernel::memory_allocate(nullptr, bytes, pool);
}
void *allocate_flags(int bytes, Dword flags, const char *name) {
    return kernel::memory_allocate(name, bytes, pool | flags);
}
void release(void *memory) {
    kernel::memory_free(memory);
}
// Placement-delete arguments after the pointer are unused.
void release_flags(void *memory, Dword, const char *) {
    kernel::memory_free(memory);
}
static void *__fastcall list_delete(List *, void *, unsigned);
const Destroy list_methods[] = {reinterpret_cast<Destroy>(list_delete)};
static void *__fastcall extra_list_delete(FocusList *, void *, unsigned);
static const Destroy extra_list_methods[] = {reinterpret_cast<Destroy>(extra_list_delete)};

static void scoped_container_destruct(Container *container) {
    container->methods = container_methods;
    try {
        container_clear(container);
    } catch (...) {
        container->methods = base_methods;
        throw;
    }
    container->methods = base_methods;
}

static void *__fastcall scoped_container_delete(Container *c, void *, unsigned flags) {
    scoped_container_destruct(c);
    if (flags & 1)
        release(c);
    return c;
}
extern const Destroy scoped_container_methods[] = {
    reinterpret_cast<Destroy>(scoped_container_delete)};

List *construct_default_list(List *list) {
    list->methods = list_methods;
    void *lock;
    try {
        lock = host->create_lock();
    } catch (...) {
        list->methods = base_methods;
        throw;
    }
    list->first = nullptr;
    list->last = nullptr;
    list->count = 0;
    list->lock = lock;
    list->granularity = 256;
    return list;
}

static void lock_scope_enter(void **stored, void *lock) {
    *stored = lock;
    if (lock)
        host->enter(lock);
}

static void lock_scope_leave(void **stored) {
    if (*stored)
        host->leave(*stored);
}
struct HostLockScope {
    void *lock;
    explicit HostLockScope(void *value) {
        lock_scope_enter(&lock, value);
    }
    ~HostLockScope() {
        lock_scope_leave(&lock);
    }
};

static Dword post_context_target(ContextState *context, Dword target, Dword event, Dword first, Dword second) {
    HostLockScope scope(context->lock);
    return target ? events::post(context, target, event, first, second, 0) : 6;
}

Dword post_target(Dword target, Dword event, Dword first, Dword second) {
    if (!target)
        return 0;
    if (context_lock) {
        HostLockScope scope(context_lock);
        if (context_state)
            return post_context_target(context_state, target, event, first, second);
    }
    return 11;
}

void list_clear(List *list) {
    list->methods = list_methods;
    try {
    {
        HostLockScope scope(list->lock);
        Object *item = list->first;
        while (item) {
            Object *next = reinterpret_cast<Object **>(item)[3];
            destroy(item);
            item = next;
        }
        list->first = nullptr;
        list->last = nullptr;
        list->count = 0;
    }
    if (list->lock)
        host->destroy_lock(list->lock);
    list->lock = nullptr;
    list->count = 0;
    list->first = nullptr;
    list->last = nullptr;
    } catch (...) {
        list->methods = base_methods;
        throw;
    }
    list->methods = base_methods;
}
static void *__fastcall list_delete(List *list, void *, unsigned flags) {
    list_clear(list);
    if (flags & 1)
        release(list);
    return list;
}

static void extra_list_destruct(FocusList *list) {
    list->methods = extra_list_methods;
    list_clear(&list->list);
    list->methods = base_methods;
}

static void *__fastcall extra_list_delete(FocusList *list, void *, unsigned flags) {
    extra_list_destruct(list);
    if (flags & 1)
        release(list);
    return list;
}
static void *__fastcall context_state_delete(ContextState *, void *, unsigned);
static const Destroy context_state_methods[] = {reinterpret_cast<Destroy>(context_state_delete)};
namespace {
template <class T, void (*Release)(T *)> class MemberCleanup {
    T *member;
public:
    explicit MemberCleanup(T *value) : member(value) {}
    ~MemberCleanup() noexcept(false) {
        if (member)
            Release(member);
    }
    void dismiss() { member = nullptr; }
};
} // namespace

static void context_state_destruct(ContextState *db) {
    MemberCleanup<Container, container_destruct> first(&db->first);
    MemberCleanup<Container, scoped_container_destruct> second(&db->second);
    MemberCleanup<List, list_clear> index(&db->index);
    MemberCleanup<Container, container_destruct> third(&db->third);
    MemberCleanup<Container, container_destruct> fourth(&db->fourth);
    db->methods = context_state_methods;
    if (db->extra)
        destroy(reinterpret_cast<Object *>(db->extra));
    host->destroy_lock(db->lock);
}

static void *__fastcall context_state_delete(ContextState *db, void *, unsigned flags) {
    context_state_destruct(db);
    if (flags & 1)
        application_release(db);
    return db;
}
static void *__fastcall registry_delete(Registry *, void *, unsigned);
static const Destroy registry_methods[] = {reinterpret_cast<Destroy>(registry_delete)};

static void registry_destruct(Registry *value) {
    value->methods = registry_methods;
    container_destruct(&value->objects);
    value->methods = base_methods;
}

static void *__fastcall registry_delete(Registry *r, void *, unsigned flags) {
    registry_destruct(r);
    if (flags & 1)
        release(r);
    return r;
}

static ContextState *context_state_construct(ContextState *db) {
    container_construct(&db->first);
    MemberCleanup<Container, container_destruct> first(&db->first);
    container_construct(&db->second);
    db->second.methods = scoped_container_methods;
    MemberCleanup<Container, scoped_container_destruct> second(&db->second);
    construct_default_list(&db->index);
    MemberCleanup<List, list_clear> index(&db->index);
    container_construct(&db->third);
    MemberCleanup<Container, container_destruct> third(&db->third);
    container_construct(&db->fourth);
    MemberCleanup<Container, container_destruct> fourth(&db->fourth);
    db->methods = context_state_methods;
    db->references = 1;
    db->lock = host->create_lock();
    db->value_50 = 0;
    db->value_58 = 0;
    auto *extra = static_cast<FocusList *>(allocate(28));
    if (extra) {
        extra->methods = base_methods;
        try {
            construct_default_list(&extra->list);
        } catch (...) {
            extra->methods = base_methods;
            release(extra);
            throw;
        }
        extra->methods = extra_list_methods;
    }
    db->extra = extra;
    fourth.dismiss();
    third.dismiss();
    index.dismiss();
    second.dismiss();
    first.dismiss();
    return db;
}

static Dword context_initialize() {
    auto *strings = static_cast<Container *>(allocate(16));
    if (strings)
        container_construct(strings);
    pooled_strings = strings;
    if (!strings)
        return 1;
    auto *context = static_cast<ContextState *>(application_allocate(112));
    if (context) {
        try {
            context_state_construct(context);
        } catch (...) {
            application_release(context);
            throw;
        }
    }
    context_state = context;
    if (!context)
        return 1;
    context_lock = host->create_lock();
    return 0;
}

static Dword context_shutdown() {
    void *lock = context_lock;
    context_lock = nullptr;
    if (pooled_strings) {
        destroy(reinterpret_cast<Object *>(pooled_strings));
        pooled_strings = nullptr;
    }
    if (context_state) {
        HostLockScope scope(lock);
        if (context_state)
            destroy(reinterpret_cast<Object *>(context_state));
        context_state = nullptr;
    }
    host->destroy_lock(lock);
    return 0;
}

static Dword scratch_initialize() {
    text_scratch = kernel::memory_allocate(nullptr, 35840, pool);
    return text_scratch ? 0 : 4;
}
static Dword scratch_shutdown() {
    if (text_scratch) {
        kernel::memory_free(text_scratch);
        text_scratch = nullptr;
    }
    return 0;
}
// (init really is XOR EAX,EAX; RET).
static Dword script_variables_initialize() {
    return 0;
}
static Dword script_variables_shutdown() {
    return container_clear(&global_script_variables);
}

static Dword objects_initialize() {
    registry_time = 0; // Update the shared widget state.
    return 0;
}
static Dword objects_shutdown() {
    return 0;
}

static Dword registry_initialize() {
    registry = static_cast<Registry *>(allocate(24));
    if (registry) {
        container_construct(&registry->objects);
        registry->methods = registry_methods;
        registry->references = 1;
    }
    return 0;
}
static Dword registry_shutdown() {
    if (registry) {
        destroy(reinterpret_cast<Object *>(registry));
        registry = nullptr;
    }
    return 0;
}

static Dword configuration_cache_initialize() {
    container_mode = 1;
    configuration_documents = static_cast<Container *>(allocate_flags(16, 0, "GContainer"));
    if (configuration_documents)
        container_construct(configuration_documents);
    string_resolver = nullptr;
    return 0;
}
static Dword configuration_cache_shutdown() {
    services::pending::gui_archive = nullptr;
    string_resolver = nullptr;
    if (configuration_documents) {
        destroy(reinterpret_cast<Object *>(configuration_documents));
        configuration_documents = nullptr;
    }
    return 0;
}

static Dword timers_initialize() {
    return 0;
}
static Dword timers_shutdown() {
    return container_clear(&timer_entries);
}
static Dword extension_shutdown() {
    return 0;
}

kernel::numeric::Extended ticks_to_seconds(int ticks) {
    alignas(16) unsigned char value[10];
    timer_x87::ticks_to_seconds_native80(ticks, value);
    return kernel::numeric::Extended::from_native80(value, kernel::numeric::current_format());
}

void tick_registered_timers() {
    timer_x87::tick_registered_timers();
}

Dword initialize(Host *callbacks, int bytes, const Settings *supplied, const char *version) {
    if (!callbacks)
        return 2;
    if (!version)
        return 17;
    if (std::strncmp("1.32.20-R", version, 10))
        return 17;
    host = callbacks;
    if (initialized)
        return 4;
    initialized = 1;
    if (!bytes && !supplied) {
        shutdown();
        return 2;
    }
    if (supplied)
        settings = *supplied;
    else
        settings = {};
    if (!(settings.flags & 1)) {
        if (!bytes) {
            shutdown();
            return 1;
        }
        pool = kernel::pool_unused();
        if (!pool) {
            shutdown();
            return 4;
        }
        backing = kernel::memory_allocate("GUI", bytes, 1024);
        if (!backing) {
            shutdown();
            return 1;
        }
        Dword alignment = settings.flags & 2 ? settings.alignment : 4,
             alternate = settings.flags & 4 ? settings.alternate_alignment : 32;
        pool_bytes = kernel::pool_create(pool, "GUI", backing, bytes, alignment, alternate, 0,
                                         false, false, true);
        if (!pool_bytes) {
            kernel::memory_free(backing);
            shutdown();
            return 1;
        }
    } else {
        if (!settings.bytes) {
            shutdown();
            return 16;
        }
        pool = settings.pool;
        pool_bytes = settings.bytes;
    }
    Dword result = scratch_initialize();
    if (!result)
        result = script_variables_initialize();
    if (!result)
        result = context_initialize();
    if (!result)
        result = objects_initialize();
    if (!result)
        result = registry_initialize();
    if (!result)
        result = configuration_cache_initialize();
    if (!result)
        result = timers_initialize();
    if (!result)
        return 0;
    shutdown();
    return result;
}
// Error returns intentionally leave the remaining state.
Dword shutdown() {
    if (!initialized)
        return 11;
    if (clipboard_text) {
        destroy(reinterpret_cast<Object *>(clipboard_text));
        clipboard_text = nullptr;
    }
    Dword result = timers_shutdown();
    if (!result)
        result = extension_shutdown();
    if (!result)
        result = configuration_cache_shutdown();
    if (!result)
        result = objects_shutdown();
    if (!result)
        result = registry_shutdown();
    if (!result)
        result = context_shutdown();
    if (!result)
        result = scratch_shutdown();
    if (!result)
        result = script_variables_shutdown();
    if (result)
        return result;

    if (!(settings.flags &
          1)) {
        kernel::memory_validate_all();
        if (kernel::memory_largest_free(pool) < pool_bytes)
            return 12;
        kernel::pool_destroy(pool);
        pool_bytes = 0;
        pool = 0;
        if (backing) {
            kernel::memory_free(backing);
            backing = nullptr;
            initialized = 0;
            return 0;
        }
        return 2;
    }
    pool = 0;
    pool_bytes = 0;
    initialized = 0;
    return 0;
}
} // namespace nfs::gui
