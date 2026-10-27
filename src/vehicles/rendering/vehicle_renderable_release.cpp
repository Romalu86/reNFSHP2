#include "vehicle_renderable_resources.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/graphics/backend/eagl.hpp"
#include <cstring>
namespace nfs::game::world::vehicle_render {
const char *const attachment_names[45] =
    {
        "Root.Root",
        "Root.WHEEL_FRONT",
        "Root.WHEEL_FRONT_LEFT",
        "Root.WHEEL_FRONT_RIGHT",
        "Root.WHEEL_REAR",
        "Root.WHEEL_REAR_LEFT",
        "Root.WHEEL_REAR_RIGHT",
        "Root.WHEEL_REAR_RIGHT_GROUND",
        "Root.WHEEL_REAR_LEFT_GROUND",
        "Root.WHEEL_FRONT_RIGHT_GROUND",
        "Root.WHEEL_FRONT_LEFT_GROUND",
        "Root.BRAKE_FRONT",
        "Root.BRAKE_FRONT_LEFT",
        "Root.BRAKE_FRONT_RIGHT",
        "Root.BRAKE_REAR",
        "Root.BRAKE_REAR_LEFT",
        "Root.BRAKE_REAR_RIGHT",
        "Root.BUMPER_FRONT",
        "Root.BUMPER_REAR",
        "Root.EXHAUST_1",
        "Root.EXHAUST_2",
        "Root.LIGHT_HEAD",
        "Root.LIGHT_HEAD_LEFT1",
        "Root.LIGHT_HEAD_RIGHT1",
        "Root.LIGHT_TAIL",
        "Root.LIGHT_TAIL_LEFT1",
        "Root.LIGHT_TAIL_LEFT2",
        "Root.LIGHT_TAIL_RIGHT1",
        "Root.LIGHT_TAIL_RIGHT2",
        "Root.DOOR_FRONT_LEFT",
        "Root.DOOR_FRONT_RIGHT",
        "Root.DOOR_REAR_LEFT",
        "Root.DOOR_FRONT_RIGHT",
        "Root.LIGHT_REVERSE1",
        "Root.LIGHT_REVERSE2",
        "Root.WINDOW_REAR_LEFT",
        "Root.WINDOW_REAR_RIGHT",
        "Root.ENGINE",
        "Root.DAMAGE02",
        "Root.TRUNK",
        "Root.PLAYER",
        "Root.LIGHT_COP_B",
        "Root.LIGHT_COP_W",
        "Root.LIGHT_COP_R",
        "Root.DAMAGE08"};

void unload_lights(void *owner) {
    if (void *array = at<void *>(owner, 744)) {
        lifecycle::construction::objects::destroy_array(
            array, 248, at<int>(static_cast<Byte *>(array) - 4, 0), world::destroy_light_glow);
        kernel::memory_free(static_cast<Byte *>(array) - 4);
        at<void *>(owner, 744) = nullptr;
    }
    at<Dword>(owner, 764) = 0;
    at<Byte>(owner, 748) = 0;
}

void pending::release_character(void *system, void *actor) {
    void *list = at<void *>(system, 92);
    if (list && at<Dword>(list, 12)) {
        world::unlink_list_node(list, actor ? offset(actor, 16) : nullptr);
        runtime::events::release(reinterpret_cast<Dword *>(offset(actor, 16)));
        if (!at<Dword>(actor, 20))
            world::invoke(actor, 1, 1u);
    }
}

void unload_driver(void *owner) {
    if (at<Byte>(owner, 12)) {
        at<Byte>(owner, 12) = 0;
        pending::release_character(subsystem(48), at<void *>(owner, 52));
        at<Dword>(owner, 52) = 0;
    }
}
// Complete RET.
void destroy_damage_zone(void *) {
}

void unload_damage(void *owner) {
    if (!at<Byte>(owner, 20))
        return;
    if (void *array = at<void *>(owner, 12)) {
        lifecycle::construction::objects::destroy_array(
            array, 20, at<int>(static_cast<Byte *>(array) - 4, 0), destroy_animation_channel);
        kernel::memory_free(static_cast<Byte *>(array) - 4);
    }
    void *table = at<void *>(owner, 64);
    at<volatile Dword>(owner, 12) = 0;
    at<volatile Dword>(owner, 16) = 0;
    if (table)
        world::invoke(table, 0, 1u);
    if (void *array = at<void *>(owner, 76)) {
        lifecycle::construction::objects::destroy_array(
            array, 72, at<int>(static_cast<Byte *>(array) - 4, 0), destroy_damage_zone);
        kernel::memory_free(static_cast<Byte *>(array) - 4);
    }
    at<volatile Dword>(owner, 76) = 0;
    at<volatile Dword>(owner, 64) = 0;
    at<volatile Byte>(owner, 20) = 0;
}

Dword pending::skeleton_node_index(void *skeleton, const char *name) {
    Dword address;
    eagl::address(at<eagl::Loader *>(skeleton, 16), "Bone", name, &address);
    return *reinterpret_cast<Dword *>(address);
}

void *pending::find_skeleton_node(void *skeleton, Dword index) {
    void *node = at<void *>(skeleton, 28);
    while (node && at<Dword>(node, 80) != index)
        node = at<void *>(node, 84);
    return node;
}

void pending::release_skeleton_node(void *pool, void *node) {
    for (unsigned index = 0; index < 256; ++index)
        if (offset(pool, 16 + index * 96) == node) {
            at<Byte>(pool, 24592 + index) = 0;
            return;
        }
}

void unload_attachments(void *owner) {
    for (unsigned index = 0; index < 45; ++index) {
        void *skeleton = at<void *>(owner, 408);
        if (at<Dword>(owner, 48 + index * 8)) {
            Dword bone = pending::skeleton_node_index(skeleton, attachment_names[index]);
            void *pool = subsystem(44);
            void *node = pending::find_skeleton_node(skeleton, bone);
            pending::release_skeleton_node(pool, node);
            at<Dword>(owner, 48 + index * 8) = 0;
        }
    }
    if (void *skeleton = at<void *>(owner, 408)) {
        world::invoke(skeleton, 0, 1u);
        at<Dword>(owner, 408) = 0;
    }
}
// Complete RET.
void unload_shadow(void *) {
}

void bind_shadow_owner(void *owner, Dword value) {
    at<Dword>(owner, 8) = value;
}

void reset_body_state(void *body) {
    std::memset(offset(body, 2076), 0, 16);
    at<Byte>(body, 2072) = 0;
}

void reset_driver(void *owner) {
    at<Dword>(owner, 44) = Dword(-1);
    at<Dword>(owner, 28) = 1;
    at<float>(owner, 32) = .5f;
    at<volatile Byte>(owner, 13) = 1;
    at<volatile Byte>(owner, 12) = 1;
    at<Dword>(owner, 16) = 0;
    pending::select_character_animation(at<void *>(owner, 52), 90902, 0);
}

float damage_amount(void *owner, int zone) {
    if (!at<Byte>(owner, 20) || zone > 10)
        return 0;
    return at<float>(at<void *>(owner, 76), 60 + Dword(zone) * 72);
}

float damage_limit(void *owner, int zone) {
    if (!at<Byte>(owner, 20) || zone > 10)
        return 0;
    return at<float>(at<void *>(owner, 76), 64 + Dword(zone) * 72);
}
} // namespace nfs::game::world::vehicle_render
namespace nfs::game::world::pending {

void unload_vehicle_renderable(void *owner) {
    using namespace vehicle_render;
    unload_lights(offset(owner, 80));
    unload_driver(offset(owner, 856));
    unload_damage(owner);
    unload_shadow(offset(owner, 3460));
    unload_attachments(offset(owner, 3480));
    world::invoke(offset(owner, 1004), 2);
    Dword skin = at<Dword>(owner, 3956), catalog = at<Dword>(owner, 3952);
    release_skin(offset(subsystem(52), 78600), catalog, skin);
    release_resource_group(offset(subsystem(52), 78600), catalog);
}

void reset_vehicle_renderable(void *owner) {
    using namespace vehicle_render;
    reset_body_state(offset(owner, 1004));
    reset_damage(owner);
    if (at<Dword>(owner, 3948) != 1 && at<Byte>(owner, 868))
        reset_driver(offset(owner, 856));
    for (unsigned channel = 0; channel < 4; ++channel)
        set_light_channel(offset(owner, 80), channel, 0);
}
} // namespace nfs::game::world::pending
