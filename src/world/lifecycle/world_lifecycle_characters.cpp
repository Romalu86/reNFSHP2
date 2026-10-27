#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/core/timing/frame_clock.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world {


void *construct_actor_list(void *list) {
    at<Dword>(list, 4) = at<Dword>(list, 8) = at<Dword>(list, 12) = 0;
    at<Byte>(list, 16) = 1;
    at<const Dword *>(list, 0) = pending::actor_list_base_methods;
    return list;
}
 
void release_actor_list_base() {
}


void clear_actor_list(void *list) {
    void *first = at<void *>(list, 4);
    while (first) {
        unlink_list_node(list, first);
        invoke<void *>(first, 0, 1u);
        first = at<void *>(list, 4);
    }
}


void initialize_characters(void *owner) {
    pending::initialize_animation_factory_entry(100000, 1);
    void *list =
        lifecycle::allocate(20, "CharacterSystem:: actorlist", lifecycle::character_pool);
    if (list) {
        construct_actor_list(list);
        at<const Dword *>(list, 0) = pending::actor_list_methods;
    }
    at<void *>(owner, 92) = list;
    lifecycle::construction::identity_matrix(reinterpret_cast<Dword *>(offset(owner, 16)));
}

void shutdown_characters(void *owner) {
    if (void *list = at<void *>(owner, 92)) {
        invoke<void *>(list, 0, 1u);
        at<void *>(owner, 92) = nullptr;
    }
    pending::shutdown_animation_factory_entry();
}

void enter_character_frontend(void *owner) {
    void *table =
        lifecycle::allocate(20, "CharacterSystem:: charTable", lifecycle::character_pool);
    if (table) {
        construct_actor_list(table);
        at<const Dword *>(table, 0) = pending::character_table_methods;
    }
    at<void *>(owner, 108) = table;
}

void leave_character_frontend(void *owner) {
    if (at<void *>(owner, 92))
        clear_actor_list(at<void *>(owner, 92));
    if (void *table = at<void *>(owner, 108)) {
        invoke<void *>(table, 0, 1u);
        at<void *>(owner, 108) = nullptr;
    }
    release_actor_list_base();
    pending::reset_animation_scratch_entry();
}

void create_character_table(void *owner) {
    void *table =
        lifecycle::allocate(20, "CharacterSystem:: charTable", lifecycle::character_pool);
    if (table) {
        construct_actor_list(table);
        at<const Dword *>(table, 0) = pending::character_table_methods;
    }
    at<void *>(owner, 108) = table;
    pending::load_character_assets(owner, 0);
}

void load_character_level(void *owner) {
    create_character_table(owner);
}
namespace {
void release_character_paths(void *owner) {
    const int count = at<int>(owner, 84);
    void **paths = at<void **>(owner, 88);
    for (int i = 0; i < count; ++i)
        gui::application_release(paths[i]);
    if (count)
        gui::application_release(paths);
    at<Dword>(owner, 84) = 0;
}
bool retire_actor_rendering(void *owner) {
    void *list = at<void *>(owner, 92);
    bool changed = false;
    for (int i = 0; list && i < at<int>(list, 12); ++i) {
        void *actor = actor_at(list, i);
        if (!at<Byte>(actor, 61)) {
            reset_actor_render_state(offset(actor, 52));
            changed = true;
        }
    }
    return changed;
}
void release_character_table(void *owner) {
    if (at<void *>(owner, 92))
        clear_actor_list(at<void *>(owner, 92));
    if (void *table = at<void *>(owner, 108)) {
        invoke<void *>(table, 0, 1u);
        at<void *>(owner, 108) = nullptr;
    }
}
void rebuild_character_table(void *owner) {
    release_actor_list_base();
    create_character_table(owner);
    pending::release_vehicle_character_bindings_entry(subsystem(52));
    at<CharacterSequenceCallback>(owner, 96)(at<void *>(owner, 100));
    at<CharacterSequenceCallback>(owner, 96) = character_frame_callback;
    frame_clock::elapsed =
        float(kernel::numeric::Extended::integer(int(graphics::unresolved::camera_state),
                                                 kernel::numeric::current_format()) *
              0.01666666753590106964111328125f);
    at<Byte>(owner, 104) = 0;
}
} // namespace

void unload_character_level(void *owner) {
    release_character_paths(owner);
    if (retire_actor_rendering(owner) && at<Byte>(owner, 104)) {
        release_character_table(owner);
        at<Dword>(owner, 84) = 0;
        rebuild_character_table(owner);
    }
    leave_character_frontend(owner);
}

void reset_character_level(void *owner) {
    if (retire_actor_rendering(owner) && at<Byte>(owner, 104)) {
        release_character_table(owner);
        release_character_paths(owner);
        rebuild_character_table(owner);
    }
}
} // namespace nfs::game::world
