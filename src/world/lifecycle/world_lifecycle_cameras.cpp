#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/core/console/console.hpp"
#include "nfs/resources/archives/archive_cache.hpp"
#include "nfs/network/session/network_virtual.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
namespace nfs::game::world {


void *construct_camera_modes(void *owner) {
    at<const Dword *>(owner, 0) = pending::camera_modes_methods;
    at<Dword>(owner, 4) = 0;
    at<Dword>(owner, 8) = 0;
    at<Dword>(owner, 12) = 0;
    at<Dword>(owner, 16) = 0;
    return owner;
}

void allocate_camera_modes(void *owner, int capacity) {
    at<int>(owner, 12) = capacity;
    at<void *>(owner, 4) = files::archives::allocate_entry(
        int(Dword(capacity) * 4), "camera controller array", lifecycle::level_pool);
}

void *construct_camera_mode_array(void *owner, int capacity) {
    at<const Dword *>(owner, 0) = pending::camera_modes_methods;
    at<void *>(owner, 4) = files::archives::allocate_entry(
        int(Dword(capacity) * 4), "camera controller array", lifecycle::level_pool);
    at<int>(owner, 12) = capacity;
    at<volatile Dword>(owner, 8) = 0;
    at<volatile Dword>(owner, 16) = 0;
    return owner;
}

void destroy_camera_modes(void *owner) {
    at<const Dword *>(owner, 0) = pending::camera_modes_methods;
    if (void *controllers = at<void *>(owner, 4))
        lifecycle::construction::network::entries::release_payload(controllers);
}

void append_camera_mode(void *owner, void *controller) {
    Dword count = at<Dword>(owner, 8) + 1;
    at<Dword>(owner, 8) = count;
    at<void **>(owner, 4)[count - 1] = controller;
}

void *detach_camera_mode(void *owner, Dword identifier) {
    int count = at<int>(owner, 8);
    void **controllers = at<void **>(owner, 4);
    void *controller = nullptr;
    for (int index = 0; index < count; ++index)
        if (at<Dword>(controllers[index], 104) == identifier) {
            controller = controllers[index];
            break;
        }
    if (!controller)
        return nullptr;
    int index = 0;
    while (index < count && controllers[index] != controller)
        ++index;
    for (int following = index + 1; following < count; ++following)
        controllers[following - 1] = controllers[following];
    controllers[count - 1] = nullptr;
    at<Dword>(owner, 8) -= 1;
    return controller;
}

void *find_camera_mode(void *owner, Dword identifier) {
    for (int index = 0; index < at<int>(owner, 8); ++index)
        if (at<Dword>(at<void **>(owner, 4)[index], 104) == identifier)
            return at<void **>(owner, 4)[index];
    return nullptr;
}

void end_camera_controller(void *owner, void *controller) {
    int active = at<int>(owner, 16);
    int index = 0;
    for (; index < active; ++index)
        if (at<void **>(owner, 4)[index] == controller)
            break;
    if (index < active) {
        void **controllers = at<void **>(owner, 4);
        void *selected = controllers[index];
        invoke(selected, 1);
        const Dword remaining = at<Dword>(owner, 16) - 1;
        at<Dword>(owner, 16) = remaining;
        active = int(remaining);
        void *saved = controllers[active];
        controllers[active] = selected;
        at<void **>(owner, 4)[index] = saved;
    }
}

void end_all_camera_modes(void *owner) {
    while (at<int>(owner, 16) != 0)
        end_camera_controller(owner, at<void **>(owner, 4)[0]);
}

void *delete_camera_modes(void *owner, Dword flags) {
    destroy_camera_modes(owner);
    if (flags & 1)
        kernel::memory_free(owner);
    return owner;
}


void *construct_camera_director(void *owner, int player) {
    construct_camera_modes(owner);
    at<const Dword *>(owner, 0) = pending::camera_director_methods;
    try {
        allocate_camera_modes(owner, 12);
    } catch (...) {
        destroy_camera_modes(owner);
        throw;
    }
    at<volatile Dword>(owner, 288) = 0;
    at<volatile Dword>(owner, 292) = 0;
    at<volatile int>(owner, 304) = player;
    at<volatile Byte>(owner, 308) = 0;
    at<volatile Dword>(owner, 356) = 0;
    at<volatile Dword>(owner, 300) = 0;
    at<volatile Byte>(owner, 164) = 0;
    at<volatile Byte>(owner, 165) = 0;
    at<volatile Byte>(owner, 166) = 0;
    at<volatile Dword>(owner, 168) = 2;
    at<volatile Byte>(owner, 172) = 0;
    at<volatile Byte>(owner, 173) = 0;
    at<volatile Byte>(owner, 277) = 0;
    at<volatile Dword>(owner, 280) = 0;
    at<volatile Dword>(owner, 160) = Dword(-1);
    at<volatile Byte>(owner, 284) = 0;
    at<volatile Byte>(owner, 285) = 0;
    at<volatile Dword>(owner, 312) = 1;
    at<volatile Dword>(owner, 460) = 0;
    at<volatile Dword>(owner, 464) = 0;
    at<volatile Dword>(owner, 468) = 0;
    at<volatile Byte>(owner, 167) = 0;
    at<volatile Dword>(owner, 296) = 0;
    return owner;
}

void destroy_camera_director(void *owner) {
    at<const Dword *>(owner, 0) = pending::camera_director_methods;
    destroy_camera_modes(owner);
}

void bind_camera_player(void *owner, int player) {
    void *cars = subsystem(68);
    int selected = at<int>(subsystem(72), 204 + player * 4);
    if (selected != -1) {
        at<void *>(owner, 300) = at<void *>(cars, 20 + selected * 4);
        return;
    }
    void *last = nullptr, *eligible = nullptr, *first_place = nullptr, *leading = nullptr;
    void **slots = reinterpret_cast<void **>(offset(cars, 20));
    for (unsigned slot = 0; slot * 84 < 2016; ++slot) {
        void *car = slots[slot];
        if (car) {
            last = car;
            void *descriptor = offset(subsystem(72), 248 + slot * 84);
            if (at<Dword>(descriptor, 0)) {
                eligible = car;
                Dword position = pending::count_racers_ahead_for_slot(subsystem(92), int(slot));
                if (position == 0)
                    first_place = car;
                else if (position == 1)
                    leading = car;
            }
        }
    }
    at<void *>(owner, 300) = leading       ? leading
                               : first_place ? first_place
                               : eligible    ? eligible
                                             : last;
}

void notify_camera_mode(void *owner) {
    unsigned player = at<int>(owner, 304) != 0;
    for (unsigned index = 0; index < 450; ++index) {
        Byte *particle = &runtime::particles::lifecycle::state<Byte>(7160496 + index * 124);
        if (at<std::uint16_t>(particle, 4) && at<Dword>(particle, 0))
            particle[112 + player] = 0;
    }
}

void enter_camera_frontend(void *) {
    void *memory = lifecycle::allocate(472, "SSCameraDirector", lifecycle::level_pool);
    tick_services[0] = memory ? construct_camera_director(memory, 0) : nullptr;
    pending::initialize_frontend_camera(tick_services[0]);
    pending::bind_frontend_camera(tick_services[0]);
}

void load_camera_level(void *) {
    const int players = network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 13);
    for (int player = 0; player < 2; ++player) {
        tick_services[player] = nullptr;
        if (player < players) {
            void *memory = lifecycle::allocate(472, "SSCameraDirector", lifecycle::level_pool);
            void *camera = memory ? construct_camera_director(memory, player) : nullptr;
            tick_services[player] = camera;
            invoke(camera, 3);
        }
    }
}

void unload_camera_level(void *) {
    kernel::print::check_console_write("camera system: start level deinit\n");
    const int players = network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 13);
    for (int player = 0; player < players; ++player) {
        if (player > 1)
            return;
        invoke(tick_services[player], 4);
        if (tick_services[player])
            invoke<void *>(tick_services[player], 0, 1u);
        tick_services[player] = nullptr;
    }
}

void reset_camera_level(void *) {
    for (int player = 0; player < at<int>(subsystem(72), 176); ++player) {
        void *camera = tick_services[player];
        while (at<int>(camera, 356) > 0) {
            end_camera_mode(camera, at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4));
            --at<Dword>(camera, 356);
            if (at<Dword>(camera, 356))
                begin_camera_mode(camera, at<Dword>(camera, 312 + at<Dword>(camera, 356) * 4));
            notify_camera_mode(camera);
        }
        at<volatile Byte>(camera, 164) = 0;
        at<volatile Byte>(camera, 165) = 0;
        at<volatile Byte>(camera, 166) = 0;
        at<volatile Dword>(camera, 168) = 2;
        at<volatile Byte>(camera, 172) = 0;
        at<volatile Byte>(camera, 173) = 0;
        at<volatile Byte>(camera, 277) = 0;
        at<volatile Dword>(camera, 280) = 0;
        at<volatile Dword>(camera, 160) = Dword(-1);
        at<volatile Byte>(camera, 285) = 0;
        at<volatile Dword>(camera, 356) = 0;
        at<volatile Dword>(camera, 300) = 0;
        bind_camera_player(tick_services[player], player);
        pending::set_camera_mode(tick_services[player], 0);
    }
    if (network::session_active())
        pending::reset_network_camera(subsystem(104));
}
} // namespace nfs::game::world
