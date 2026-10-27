#include "animation_resource_runtime.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::actor_resources {
namespace {
struct ResourceCleanup {
    void *object;
    void (*destroy)(void *);
    ~ResourceCleanup() noexcept(false) {
        if (destroy)
            destroy(object);
    }
    void dismiss() { destroy = nullptr; }
};
} // namespace

void *construct_node(void *node) {
    lifecycle::construction::objects::event_member(node);
    at<Dword>(node, 8) = at<Dword>(node, 12) = at<Dword>(node, 16) = 0;
    at<const Dword *>(node, 0) = node_methods;
    return node;
}

void destroy_node(void *node) {
    at<const Dword *>(node, 0) = node_methods;
    at<Dword>(node, 8) = at<Dword>(node, 12) = 0;
    lifecycle::construction::objects::event_member_destroy(node);
}

void *__fastcall delete_node(void *node, void *, Dword flags) {
    destroy_node(node);
    if (flags & 1)
        kernel::memory_free(node);
    return node;
}

const char *__fastcall node_name(void *, void *) {
    return "ListNode";
}

void destroy_animation_entry(void *entry) {
    destroy_node(entry);
}

void *__fastcall delete_animation_entry(void *entry, void *, Dword flags) {
    destroy_animation_entry(entry);
    if (flags & 1)
        kernel::memory_free(entry);
    return entry;
}

const char *__fastcall animation_entry_name(void *entry, void *) {
    return reinterpret_cast<const char *>(offset(entry, 32));
}

void destroy_animation_table(void *table) {
    at<const Dword *>(table, 0) = animation_table_methods;
    at<const Dword *>(table, 20) = animation_reference_methods;
    ResourceCleanup base{table, dependency::destroy_actor_list_base};
    ResourceCleanup reference{table ? offset(table, 20) : nullptr,
                              lifecycle::construction::objects::event_member_destroy};
    while (void *entry = at<void *>(table, 4)) {
        void *animation = at<void *>(entry, 24);
        world::invoke(animation, 0, 0u);
        auto type = at<std::uint16_t>(animation, 8);
        at<Dword>(animation, 0) = dependency::animation_state.scratch[type];
        dependency::animation_state.scratch[type] = reinterpret_cast<Dword>(animation);
        world::unlink_list_node(table, entry);
        runtime::events::release(static_cast<Dword *>(entry));
        if (!at<Dword>(entry, 4))
            world::invoke(entry, 0, 1u);
    }
    if (auto *loader = at<eagl::Loader *>(table, 28)) {
        eagl::loader_destroy(loader);
        eagl::deallocator(loader, 28);
    }
    kernel::memory_free(at<void *>(table, 32));
}

void *__fastcall delete_animation_table(void *table, void *, Dword flags) {
    destroy_animation_table(table);
    if (flags & 1)
        kernel::memory_free(table);
    return table;
}

void *__fastcall delete_animation_reference(void *reference, void *, Dword flags) {
    return delete_animation_table(static_cast<Byte *>(reference) - 20, nullptr, flags);
}
void __fastcall append_animation_entry(void *table, void *, void *entry) {
    dependency::append_actor_node(table, entry);
}
 
const Dword node_methods[2] = {reinterpret_cast<Dword>(delete_node),
                              reinterpret_cast<Dword>(node_name)};
const Dword animation_entry_methods[2] = {reinterpret_cast<Dword>(delete_animation_entry),
                                         reinterpret_cast<Dword>(animation_entry_name)};
const Dword animation_table_methods[2] = {reinterpret_cast<Dword>(delete_animation_table),
                                         reinterpret_cast<Dword>(append_animation_entry)};
const Dword animation_reference_methods[1] = {reinterpret_cast<Dword>(delete_animation_reference)};

void __fastcall bind_animation_bank(void *factory, void *, void *bank) {
    at<void *>(factory, 4) = bank;
}

Dword __fastcall animation_bank_count(void *factory, void *) {
    return at<Dword>(at<void *>(factory, 4), 4);
}

void *__fastcall named_bank_animation(void *factory, void *, const char *name) {
    void *bank = at<void *>(factory, 4);
    int index = bank_animation_index(bank, nullptr, name);
    return index == -1 ? nullptr : at<void **>(bank, 12)[index];
}

void *__fastcall indexed_bank_animation(void *factory, void *, int index) {
    return at<void **>(at<void *>(factory, 4), 12)[index];
}

const char *__fastcall bank_animation_name(void *factory, void *, int index) {
    return at<const char **>(at<void *>(factory, 4), 16)[index];
}

int __fastcall find_bank_animation(void *factory, void *, const char *name) {
    return bank_animation_index(at<void *>(factory, 4), nullptr, name);
}

Dword __fastcall bank_animation_flags(void *factory, void *) {
    return at<Dword>(at<void *>(factory, 4), 8);
}

void *__fastcall delete_bank_factory(void *factory, void *, Dword flags) {
    at<const Dword *>(factory, 0) = animation_bank_methods;
    if (flags & 1)
        kernel::memory_free(factory);
    return factory;
}
const Dword animation_bank_methods[8] = {reinterpret_cast<Dword>(bind_animation_bank),
                                        reinterpret_cast<Dword>(animation_bank_count),
                                        reinterpret_cast<Dword>(named_bank_animation),
                                        reinterpret_cast<Dword>(indexed_bank_animation),
                                        reinterpret_cast<Dword>(bank_animation_name),
                                        reinterpret_cast<Dword>(find_bank_animation),
                                        reinterpret_cast<Dword>(bank_animation_flags),
                                        reinterpret_cast<Dword>(delete_bank_factory)};

void __fastcall read_animation_type(const void *animation, void *, std::uint16_t *type) {
    *type = *static_cast<const std::uint16_t *>(animation);
}

void *__stdcall create_animation(std::uint16_t type) {
    return pending::create_runtime_animation(type);
}
namespace {
void read_bank(void *table, const char *resource, Dword directory, Dword pool, bool damage) {
    std::strcpy(reinterpret_cast<char *>(offset(table, 40)), resource);
    char path[260];
    std::sprintf(path, "%s%s\\%s", offset(subsystem(4), directory * 260 + 4), resource,
                 "animbank.o");
    auto *loader = graphics::load_module(path, resolve_animation_symbol,
                                         reinterpret_cast<void **>(offset(table, 32)), pool);
    at<eagl::Loader *>(table, 28) = loader;
    if (!loader)
        return;
    int cursor = 0;
    eagl::next_address(loader, "AnimationBank", &cursor,
                       reinterpret_cast<Dword *>(offset(table, 36)));
    void *bank = at<void *>(table, 36);
    for (int index = 0; index < at<int>(bank, 4); ++index) {
        void *entry = lifecycle::allocate(68, "anim table entry", pool);
        if (entry) {
            try {
                construct_node(entry);
            } catch (...) {
                kernel::memory_free(entry);
                throw;
            }
            at<Dword>(entry, 24) = at<Dword>(entry, 28) = 0;
            at<const Dword *>(entry, 0) = animation_entry_methods;
        }
        void *raw = at<void **>(bank, 12)[index];
        std::uint16_t type;
        read_animation_type(raw, nullptr, &type);
        const char *name = at<const char **>(bank, 16)[index];
        std::size_t length = std::strlen(name);
        void *animation = create_animation(*static_cast<std::uint16_t *>(raw));
        if (animation)
            world::invoke(animation, 15, raw);
        at<void *>(entry, 24) = animation;
        if (damage && !world::invoke<Byte>(animation, 4, offset(entry, 20)))
            at<Dword>(entry, 20) = 1132396544;
        Dword identifier = 0;
        char suffix = name[length - 1];
        const Dword increment = 1000000;
        if (suffix == 'q')
            std::sscanf(name, damage ? "anim_damage%d_q" : "t%d_q", &identifier);
        else if (suffix == 't') {
            std::sscanf(name, damage ? "anim_damage%d_t" : "t%d_t", &identifier);
            identifier += increment;
        } else if (suffix == 'e' || type == 1) {
            std::sscanf(name, suffix == 'e' ? (damage ? "anim_damage%d_e" : "t%d_e") : "E_t%d",
                        &identifier);
            identifier += increment * 2;
        }
        at<Dword>(entry, 28) = identifier;
        std::sprintf(reinterpret_cast<char *>(offset(entry, 32)), "AnimTableEntry(%d)",
                     int(identifier));
        runtime::events::acquire(static_cast<Dword *>(entry));
        world::invoke(table, 1, entry);
    }
}
} // namespace

void load_animation_table(void *table, const char *resource, Dword directory, Dword pool) {
    read_bank(table, resource, directory, pool, false);
}
} // namespace nfs::game::world::actor_resources
namespace nfs::game::world::vehicle_render::pending {

void *construct_damage_animation_table(void *table) {
    world::construct_actor_list(table);
    actor_resources::ResourceCleanup base{table, dependency::destroy_actor_list_base};
    lifecycle::construction::objects::event_member(offset(table, 20));
    actor_resources::ResourceCleanup reference{
        offset(table, 20), lifecycle::construction::objects::event_member_destroy};
    at<Dword>(table, 28) = at<Dword>(table, 32) = at<Dword>(table, 36) = 0;
    at<const Dword *>(table, 0) = actor_resources::animation_table_methods;
    at<const Dword *>(table, 20) = actor_resources::animation_reference_methods;
    reference.dismiss();
    base.dismiss();
    return table;
}

void load_damage_animation_table(void *table, const char *resource, Dword directory, Dword pool) {
    actor_resources::read_bank(table, resource, directory, pool, true);
}
} // namespace nfs::game::world::vehicle_render::pending
