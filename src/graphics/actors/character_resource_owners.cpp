#include "../animation/animation_resource_runtime.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::vehicle_render {
void *__fastcall delete_skeleton(void *, void *, Dword);
}
namespace nfs::game::world::actor_resources {
namespace {
// Member/base unwind actions also execute when a later resource call throws.
template <class Destroy> struct ResourceCleanup {
    void *object;
    Destroy destroy;
    ~ResourceCleanup() noexcept(false) {
        if (destroy)
            destroy(object);
    }
    void dismiss() { destroy = nullptr; }
};
template <class Destroy> ResourceCleanup(void *, Destroy) -> ResourceCleanup<Destroy>;
extern const Dword model_base_methods[1], model_methods[1], model_reference_methods[2];
extern const Dword texture_list_base_methods[2], texture_list_methods[2];
extern const Dword character_base_methods[2], character_methods[2], character_reference_methods[2],
    texture_methods[2];
Dword character_sequence{};
} // namespace

void release_skeleton(void *object) {
    vehicle_render::delete_skeleton(object, nullptr, 0);
}

void load_skeleton_file(void *object, const char *path, Dword pool) {
    auto *loader = graphics::load_module(path, graphics::symbols::resolve_module,
                                         reinterpret_cast<void **>(offset(object, 20)), pool);
    at<eagl::Loader *>(object, 16) = loader;
    int cursor = 0;
    eagl::next_address(loader, "Skeleton", &cursor, reinterpret_cast<Dword *>(offset(object, 24)));
    at<Byte>(object, 112) = 1;
}

void destroy_model_base(void *object) {
    at<const Dword *>(object, 0) = model_base_methods;
    at<Dword>(object, 128) = 0;
    release_skeleton(object);
}
namespace {
void clear_resource_list(void *list) {
    ResourceCleanup base{list, dependency::destroy_actor_list_base};
    void *entry = at<void *>(list, 4);
    while (entry) {
        void *next = at<void *>(entry, 12);
        world::unlink_list_node(list, entry);
        if (!at<Dword>(entry, 4))
            world::invoke(entry, 0, 1u);
        entry = next;
    }
}
} // namespace

void destroy_texture_list(void *list) {
    at<const Dword *>(list, 0) = texture_list_base_methods;
    clear_resource_list(list);
}

void destroy_character_base(void *list) {
    at<const Dword *>(list, 0) = character_base_methods;
    clear_resource_list(list);
}

void destroy_model_entry(void *object) {
    ResourceCleanup base{object, destroy_model_base};
    ResourceCleanup textures{object ? offset(object, 160) : nullptr, destroy_texture_list};
    destroy_node(object ? offset(object, 180) : nullptr);
}

void destroy_character_entry(void *object) {
    at<const Dword *>(object, 0) = character_methods;
    at<const Dword *>(object, 20) = character_reference_methods;
    ResourceCleanup base{object, destroy_character_base};
    ResourceCleanup node{object ? offset(object, 20) : nullptr, destroy_node};
    destroy_animation_table(offset(object, 40));
}

void destroy_character_texture(void *object) {
    at<const Dword *>(object, 0) = texture_methods;
    ResourceCleanup node{object, destroy_node};
    kernel::memory_free(at<void *>(object, 20));
}

void *__fastcall delete_model_base(void *object, void *, Dword flags) {
    destroy_model_base(object);
    if (flags & 1)
        kernel::memory_free(object);
    return object;
}

void *__fastcall delete_model_entry(void *object, void *, Dword flags) {
    destroy_model_entry(object);
    if (flags & 1)
        kernel::memory_free(object);
    return object;
}

void *__fastcall delete_model_texture_list(void *list, void *, Dword flags) {
    return delete_model_entry(static_cast<Byte *>(list) - 160, nullptr, flags);
}

void *__fastcall delete_model_reference(void *reference, void *, Dword flags) {
    return delete_model_entry(static_cast<Byte *>(reference) - 180, nullptr, flags);
}

const char *__fastcall model_entry_name(void *, void *) {
    return "modelTableEntry";
}

void *__fastcall delete_texture_list(void *list, void *, Dword flags) {
    destroy_texture_list(list);
    if (flags & 1)
        kernel::memory_free(list);
    return list;
}

void *__fastcall delete_character_base(void *object, void *, Dword flags) {
    destroy_character_base(object);
    if (flags & 1)
        kernel::memory_free(object);
    return object;
}

void *__fastcall delete_character_entry(void *object, void *, Dword flags) {
    destroy_character_entry(object);
    if (flags & 1)
        kernel::memory_free(object);
    return object;
}

void *__fastcall delete_character_reference(void *reference, void *, Dword flags) {
    return delete_character_entry(static_cast<Byte *>(reference) - 20, nullptr, flags);
}

const char *__fastcall character_entry_name(void *reference, void *) {
    return reinterpret_cast<const char *>(offset(reference, 76));
}

void *__fastcall delete_character_texture(void *texture, void *, Dword flags) {
    destroy_character_texture(texture);
    if (flags & 1)
        kernel::memory_free(texture);
    return texture;
}

const char *__fastcall character_texture_name(void *, void *) {
    return "textureFile";
}
namespace {
void __fastcall append_resource(void *list, void *, void *entry) {
    dependency::append_actor_node(list, entry);
}
const Dword model_base_methods[1] = {reinterpret_cast<Dword>(delete_model_base)};
const Dword model_methods[1] = {reinterpret_cast<Dword>(delete_model_entry)};
const Dword model_reference_methods[2] = {reinterpret_cast<Dword>(delete_model_reference),
                                         reinterpret_cast<Dword>(model_entry_name)};
const Dword texture_list_base_methods[2] = {reinterpret_cast<Dword>(delete_texture_list),
                                           reinterpret_cast<Dword>(append_resource)};
const Dword texture_list_methods[2] = {reinterpret_cast<Dword>(delete_model_texture_list),
                                      reinterpret_cast<Dword>(append_resource)};
const Dword character_base_methods[2] = {reinterpret_cast<Dword>(delete_character_base),
                                        reinterpret_cast<Dword>(append_resource)};
const Dword character_methods[2] = {reinterpret_cast<Dword>(delete_character_entry),
                                   reinterpret_cast<Dword>(append_resource)};
const Dword character_reference_methods[2] = {
    reinterpret_cast<Dword>(delete_character_reference),
    reinterpret_cast<Dword>(character_entry_name)};
const Dword texture_methods[2] = {reinterpret_cast<Dword>(delete_character_texture),
                                 reinterpret_cast<Dword>(character_texture_name)};
} // namespace

void *construct_character_entry(void *object) {
    world::construct_actor_list(object);
    at<const Dword *>(object, 0) = character_base_methods;
    ResourceCleanup base{object, destroy_character_base};
    construct_node(offset(object, 20));
    ResourceCleanup node{offset(object, 20), destroy_node};
    vehicle_render::pending::construct_damage_animation_table(offset(object, 40));
    ResourceCleanup animations{offset(object, 40), destroy_animation_table};
    at<Dword>(object, 128) = character_sequence++;
    at<const Dword *>(object, 0) = character_methods;
    at<const Dword *>(object, 20) = character_reference_methods;
    std::sprintf(reinterpret_cast<char *>(offset(object, 96)), "characterTableEntry(%d)",
                 int(at<Dword>(object, 128)));
    animations.dismiss();
    node.dismiss();
    base.dismiss();
    return object;
}

void *acquire_character(void *table, const char *resource) {
    for (void *node = at<void *>(table, 4); node; node = at<void *>(node, 12))
        if (!std::strcmp(resource, reinterpret_cast<const char *>(offset(node, 60)))) {
            void *character = static_cast<Byte *>(node) - 20;
            if (character)
                return character;
            break;
        }
    void *character = lifecycle::allocate(132, "characterTableEntry", lifecycle::character_pool);
    if (character) {
        try {
            construct_character_entry(character);
        } catch (...) {
            kernel::memory_free(character);
            throw;
        }
    }
    load_animation_table(offset(character, 40), resource, 3, lifecycle::character_pool);
    world::invoke(table, 1, character ? offset(character, 20) : nullptr);
    return character;
}

void *acquire_character_model(void *character, const char *resource, const char *variant) {
    for (void *node = at<void *>(character, 4); node; node = at<void *>(node, 12)) {
        void *model = static_cast<Byte *>(node) - 180;
        if (!std::strcmp(variant, reinterpret_cast<const char *>(offset(model, 132))))
            return model;
    }
    void *model = lifecycle::allocate(208, "modelTableEntry", lifecycle::character_pool);
    if (model) {
        ResourceCleanup allocation{model, kernel::memory_free};
        vehicle_render::construct_skeleton(model);
        ResourceCleanup skeleton{model, release_skeleton};
        at<Dword>(model, 128) = at<Dword>(model, 148) = 0;
        at<const Dword *>(model, 0) = model_base_methods;
        skeleton.destroy = destroy_model_base;
        world::construct_actor_list(offset(model, 160));
        at<const Dword *>(model, 160) = texture_list_base_methods;
        ResourceCleanup textures{offset(model, 160), destroy_texture_list};
        construct_node(offset(model, 180));
        ResourceCleanup node{offset(model, 180), destroy_node};
        at<const Dword *>(model, 160) = texture_list_methods;
        at<const Dword *>(model, 180) = model_reference_methods;
        at<const Dword *>(model, 0) = model_methods;
        std::strcpy(reinterpret_cast<char *>(offset(model, 132)), variant);
        char path[260];
        std::sprintf(path, "%s%s\\%s\\model.o", offset(subsystem(4), 784), resource, variant);
        load_skeleton_file(model, path, lifecycle::character_pool);
        int cursor = 0;
        eagl::next_address(at<eagl::Loader *>(model, 16), "Model", &cursor,
                           reinterpret_cast<Dword *>(offset(model, 128)));
        node.dismiss();
        textures.dismiss();
        skeleton.dismiss();
        allocation.dismiss();
    }
    world::invoke(character, 1, model ? offset(model, 180) : nullptr);
    return model;
}

void *acquire_character_texture(void *list, const char *resource, const char *variant,
                                const char *filename) {
    for (void *node = at<void *>(list, 4); node; node = at<void *>(node, 12))
        if (!std::strcmp(filename, reinterpret_cast<const char *>(offset(node, 24))))
            return node;
    void *texture = lifecycle::allocate(40, "texture", lifecycle::character_pool);
    if (texture) {
        ResourceCleanup allocation{texture, kernel::memory_free};
        construct_node(texture);
        ResourceCleanup node{texture, destroy_node};
        at<Dword>(texture, 20) = 0;
        at<const Dword *>(texture, 0) = texture_methods;
        std::strcpy(reinterpret_cast<char *>(offset(texture, 24)), filename);
        char path[260];
        std::sprintf(path, "%s%s\\%s\\%s", offset(subsystem(4), 784), resource, variant,
                     filename);
        Dword count;
        runtime::particles::lifecycle::load_fsh_file(path, &count,
                                                     reinterpret_cast<Dword **>(offset(texture, 20)),
                                                     lifecycle::character_pool);
        node.dismiss();
        allocation.dismiss();
    }
    world::invoke(list, 1, texture);
    return texture;
}
} // namespace nfs::game::world::actor_resources
