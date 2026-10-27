#include "nfs/world/lifecycle/world_dependency.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/network/session/network_virtual.hpp"
namespace nfs::game::world::dependency {

void insert_actor_node(void *list, void *previous, void *node) {
    void *head = at<void *>(list, 4);
    for (void *scan = head; scan; scan = at<void *>(scan, 12)) {
    }
    at<void *>(node, 12) = at<void *>(node, 8) = nullptr;
    if (!previous) {
        if (!head)
            at<void *>(list, 8) = node;
        else {
            at<void *>(head, 8) = node;
            at<void *>(node, 12) = head;
        }
        at<void *>(list, 4) = node;
    } else {
        void *next = at<void *>(previous, 12);
        at<void *>(node, 8) = previous;
        at<void *>(node, 12) = next;
        at<void *>(previous, 12) = node;
        if (next)
            at<void *>(next, 8) = node;
        if (previous == at<void *>(list, 8))
            at<void *>(list, 8) = node;
    }
    Dword count = at<Dword>(list, 12) + 1;
    at<Dword>(list, 12) = count;
    void *first = at<void *>(list, 4);
    for (void *scan = first; scan; scan = at<void *>(scan, 12)) {
    }
    if (at<Byte>(list, 16) && count)
        for (void *scan = first; scan; scan = at<void *>(scan, 12)) {
        }
}

void append_actor_node(void *list, void *node) {
    insert_actor_node(list, at<void *>(list, 8), node);
}

void destroy_actor_list_base(void *list) {
    at<const Dword *>(list, 0) = world::pending::actor_list_base_methods;
    void *node = at<void *>(list, 4);
    while (node) {
        void *next = at<void *>(node, 12);
        if (at<int>(node, 4) < 1)
            world::invoke<void *>(node, 0, 1u);
        node = next;
    }
    at<Dword>(list, 8) = at<Dword>(list, 4) = at<Dword>(list, 12) = 0;
}

void destroy_actor_list(void *list) {
    at<const Dword *>(list, 0) = world::pending::actor_list_methods;
    while (void *node = at<void *>(list, 4))
        world::unlink_list_node(list, node);
    destroy_actor_list_base(list);
}

void destroy_character_table(void *list) {
    at<const Dword *>(list, 0) = world::pending::character_table_methods;
    void *node = at<void *>(list, 4);
    while (node) {
        void *next = at<void *>(node, 12);
        world::unlink_list_node(list, node);
        if (at<Dword>(node, 4) == 0)
            world::invoke<void *>(node, 0, 1u);
        node = next;
    }
    destroy_actor_list_base(list);
}

void *delete_actor_list_base(void *list, Dword flags) {
    destroy_actor_list_base(list);
    if (flags & 1)
        kernel::memory_free(list);
    return list;
}

void *delete_actor_list(void *list, Dword flags) {
    destroy_actor_list(list);
    if (flags & 1)
        kernel::memory_free(list);
    return list;
}

void *delete_character_table(void *list, Dword flags) {
    destroy_character_table(list);
    if (flags & 1)
        kernel::memory_free(list);
    return list;
}

void *delete_start_line(void *owner, Dword flags) {
    at<const Dword *>(owner, 0) = world::pending::start_line_methods;
    if (void *next = at<void *>(owner, 8)) {
        world::invoke<void *>(next, 0, 1u);
        at<void *>(owner, 8) = nullptr;
    }
    if (void *geometry = at<void *>(owner, 4)) {
        world::invoke<void *>(geometry, 0, 1u);
        at<void *>(owner, 4) = nullptr;
    }
    at<void *>(owner, 8) = nullptr;
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void *delete_base_boom(void *owner, Dword flags) {
    at<const Dword *>(owner, 0) = world::pending::boom_base_methods;
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void *delete_boom_array(void *owner, Dword flags) {
    if (flags & 2) {
        auto *cookie = static_cast<Dword *>(owner) - 1;
        lifecycle::construction::objects::destroy_array(owner, 120, int(*cookie),
                                                        world::destroy_boom);
        if (flags & 1)
            lifecycle::construction::network::entries::release_payload(cookie);
        return cookie;
    }
    world::destroy_boom(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void *delete_camera_director(void *owner, Dword flags) {
    world::destroy_camera_director(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}

void read_boom_position(void *owner, Dword *out) {
    float z = at<float>(owner, 36);
    out[0] = at<Dword>(owner, 28);
    reinterpret_cast<float *>(out)[2] = -z;
    out[1] = at<Dword>(owner, 32);
}

void *boom_definition(void *owner) {
    return at<void *>(at<void *>(at<void *>(owner, 76), 32), 0);
}

void read_boom_bounds(void *owner, Dword *out) {
    float first_z = at<float>(owner, 48);
    Dword first_x = at<Dword>(owner, 40);
    reinterpret_cast<float *>(out)[2] = -first_z;
    out[0] = first_x;
    float second_z = at<float>(owner, 60);
    Dword first_y = at<Dword>(owner, 44);
    out[1] = first_y;
    Dword second_x = at<Dword>(owner, 52);
    reinterpret_cast<float *>(out)[5] = -second_z;
    float third_x = at<float>(owner, 64);
    out[3] = second_x;
    Dword second_y = at<Dword>(owner, 56);
    reinterpret_cast<float *>(out)[6] = -third_x;
    out[4] = second_y;
    float third_y = at<float>(owner, 68);
    Dword third_z = at<Dword>(owner, 72);
    out[8] = third_z;
    reinterpret_cast<float *>(out)[7] = -third_y;
}

void *current_camera_state(void *owner) {
    if (at<Dword>(owner, 356))
        if (void *controller =
                world::find_camera_mode(owner, at<Dword>(owner, 312 + at<Dword>(owner, 356) * 4)))
            return offset(controller, 4);
    return nullptr;
}
} // namespace nfs::game::world::dependency
