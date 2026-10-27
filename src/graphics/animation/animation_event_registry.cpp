#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "animation_resource_runtime.hpp"
#include <cstring>
#include <cstdlib>
namespace nfs::game::world::actor_resources {
static_assert(sizeof(EventRegistry) == 24 && offsetof(EventRegistry, mapping) == 8);
ActorEventStorage actor_event_storage{};
EventRegistry &animation_events = actor_event_storage.registry;

int find_animation_event(EventRegistry *registry, const char *name, int *insertion) {
    int upper = int(registry->count) - 1;
    *insertion = 0;
    while (upper >= *insertion) {
        int middle = int(Dword(*insertion) + Dword(upper)) >> 1;
        int order = std::strcmp(name, registry->mapping[middle].name);
        if (!order)
            return int(registry->mapping[middle].identifier);
        if (order < 0)
            upper = middle - 1;
        else
            *insertion = middle + 1;
    }
    return -1;
}

Byte register_animation_event(EventRegistry *registry, const char *symbol, Dword *identifier) {
    if (!symbol || std::strstr(symbol, "event.") != symbol)
        return 0;
    const char *name = symbol + 6;
    int insertion;
    *identifier = Dword(find_animation_event(registry, name, &insertion));
    if (int(*identifier) >= 0)
        return 1;
    int old_capacity = registry->capacity;
    if (int(registry->count) + 1 < old_capacity) {
        for (int index = registry->count; index >= insertion; --index)
            if (index > 0)
                registry->mapping[index] = registry->mapping[index - 1];
        registry->mapping[insertion].name =
            static_cast<char *>(eagl::allocator(Dword(std::strlen(name)) + 1, "eventName"));
        std::strcpy(registry->mapping[insertion].name, name);
        registry->names[registry->count] = registry->mapping[insertion].name;
        registry->mapping[insertion].identifier = registry->count;
    } else {
        char **old_names = registry->names;
        EventMapping *old_mapping = registry->mapping;
        registry->capacity = int(Dword(old_capacity) + Dword(registry->growth));
        registry->mapping = static_cast<EventMapping *>(
            eagl::allocator(Dword(registry->capacity) * 8, "eventMapping"));
        registry->names =
            static_cast<char **>(eagl::allocator(Dword(registry->capacity) * 4, "eventNames"));
        for (int index = registry->count; index >= insertion; --index)
            if (index > 0)
                registry->mapping[index] = old_mapping[index - 1];
        for (int index = insertion - 1; index >= 0; --index)
            registry->mapping[index] = old_mapping[index];
        registry->mapping[insertion].name =
            static_cast<char *>(eagl::allocator(Dword(std::strlen(name)) + 1, "eventName"));
        std::strcpy(registry->mapping[insertion].name, name);
        registry->names[registry->count] = registry->mapping[insertion].name;
        for (unsigned index = 0; index < registry->count; ++index)
            registry->names[index] = old_names[index];
        registry->mapping[insertion].identifier = registry->count;
        if (old_mapping)
            eagl::deallocator(old_mapping, Dword(old_capacity) * 8);
        if (old_names)
            eagl::deallocator(old_names, Dword(old_capacity) * 4);
    }
    ++registry->count;
    *identifier = Dword(registry->count) - 1;
    return 1;
}

void release_animation_events(EventRegistry *registry) {
    if (registry->mapping) {
        for (unsigned index = 0; index < registry->count; ++index) {
            char *name = registry->mapping[index].name;
            if (name)
                eagl::deallocator(name, Dword(std::strlen(name)) + 1);
        }
        eagl::deallocator(registry->mapping, Dword(registry->capacity) * 8);
    }
    if (registry->names)
        eagl::deallocator(registry->names, Dword(registry->capacity) * 4);
}

void shutdown_animation_events() {
    release_animation_events(&animation_events);
}
 
void initialize_animation_events() {
    animation_events.count = 0;
    animation_events.mapping = nullptr;
    animation_events.names = nullptr;
    animation_events.capacity = 0;
    animation_events.growth = 50;
    std::atexit(shutdown_animation_events);
}
// Compare all twenty bytes before writing the found flag.
Dword resolve_animation_symbol(const char *name, Byte *found) {
    constexpr char buffer_symbol[] = "EAGLAnimationBuffer";
    static_assert(sizeof(buffer_symbol) == 20);
    unsigned compared = 0;
    while (compared < 20 && name[compared] == buffer_symbol[compared])
        ++compared;
    *found = 1;
    if (compared == 20)
        return 1879048192;
    Dword value = 0;
    if (register_animation_event(&animation_events, name, &value))
        return value;
    *found = 0;
    return 0;
}

int __fastcall bank_animation_index(void *bank, void *, const char *name) {
    int lower = 0, upper = int(at<Dword>(bank, 4) - 1u);
    while (lower <= upper) {
        int middle = int(Dword(lower) + Dword(upper)) >> 1;
        int order = std::strcmp(name, at<const char **>(bank, 16)[middle]);
        if (!order)
            return middle;
        if (order < 0)
            upper = middle - 1;
        else
            lower = middle + 1;
    }
    return -1;
}
} // namespace nfs::game::world::actor_resources
NFS_KERNEL_INITIALIZER(startup_initialize_animation_events, ".CRT$XCU_NFS_00690004",
                       nfs::game::world::actor_resources::initialize_animation_events);
