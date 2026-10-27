#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "nfs/audio/vehicles/vehicle_audio.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/core/services/service_lifecycle.hpp"
#include <cstring>
namespace nfs::network::packets::pending {
void reset_lobby();
}
namespace nfs::game::world {

void load_car_level(void *owner) {
    if (network::session_active() && at<std::int16_t>(subsystem(104), 10) < 4)
        network::packets::pending::reset_lobby();
    pending::load_ai_path_groups(subsystem(76));
    pending::assign_start_grid(owner);
    pending::refresh_car_routes(owner);
    at<Dword>(owner, 696) = 32;
    at<Dword>(owner, 692) = 64;
}

void unload_car_level(void *owner) {
    kernel::signal_wait(reinterpret_cast<kernel::Signal *>(offset(subsystem(104), 84)));
    for (unsigned index = 0; index < 24; ++index) {
        void *car = at<void *>(owner, 20 + index * 4);
        if (car) {
            pending::destroy_car(car);
            gui::application_release(car);
            at<void *>(owner, 20 + index * 4) = nullptr;
        }
    }
    kernel::signal_set(reinterpret_cast<kernel::Signal *>(offset(subsystem(104), 84)));
    at<Dword>(owner, 4) = at<Dword>(owner, 8) = at<Dword>(owner, 12) = at<Dword>(owner, 16) = 0;
}

void enter_car_frontend(void *owner) {
    at<Dword>(owner, 4) = at<Dword>(owner, 8) = at<Dword>(owner, 12) = at<Dword>(owner, 16) = 0;
    void *memory = lifecycle::allocate(8004, "CarManager::LoadAllCars", lifecycle::cars_pool);
    void *car = memory ? pending::construct_car(memory) : nullptr;
    at<void *>(owner, 20) = car;
    kernel::memory_zero(car, 8004);
}

void reset_car_level(void *owner) {
    pending::assign_start_grid(owner);
    pending::refresh_car_routes(owner);
    if (network::session_active() &&
        (at<std::int16_t>(subsystem(104), 10) < 4 || at<int>(subsystem(72), 168) < 1)) {
        Dword x = at<Dword>(owner, 784), z = at<Dword>(owner, 792);
        float y = at<float>(owner, 788) + 2.0f;
        for (void *camera : tick_services)
            if (camera) {
                at<Dword>(camera, 288) = x;
                at<float>(camera, 292) = y;
                at<Dword>(camera, 296) = z;
            }
        return;
    }
    int count = at<int>(owner, 4);
    for (int index = 0; index < count; ++index) {
        void *car = at<void *>(owner, 20 + index * 4);
        pending::reset_car_start_position(car);
        if (audio::vehicle::car_audio_suppressed(car)) {
            void *camera = tick_services[at<int>(car, 1776)];
            at<Dword>(camera, 288) = at<Dword>(owner, 700 + index * 12);
            at<Dword>(camera, 292) = at<Dword>(owner, 704 + index * 12);
            at<Dword>(camera, 296) = at<Dword>(owner, 708 + index * 12);
        }
    }
}
// The caller must provide a valid allocation and an available slot.
void *create_car(void *owner, const char *requested_name, Dword variant, Dword client, Dword options,
                 Dword flags, Dword requested_slot) {
    Dword slot = requested_slot;
    if (slot == 4294967295u) {
        for (Dword index = 0; index < 24; ++index)
            if (!at<void *>(owner, 20 + index * 4)) {
                slot = index;
                break;
            }
    }
    void *memory = lifecycle::allocate(8004, "CarManager::LoadCar", lifecycle::cars_pool);
    void *car = memory ? pending::construct_car(memory) : nullptr;
    at<void *>(owner, 20 + slot * 4) = car;
    at<Dword>(car, 7988) = client;
    pending::register_car_particles(car);
    void *rules = subsystem(72);
    char name[12];
    Dword catalog;
    if (!network::session_active()) {
        std::strncpy(name, requested_name, 12);
        catalog = Dword(services::find_catalog_car(offset(subsystem(52), 4), name));
        void *descriptor = slot < 24 ? offset(rules, 248 + slot * 84) : nullptr;
        at<Dword>(descriptor, 60) = catalog;
    } else {
        void *descriptor = slot < 24 ? offset(rules, 248 + slot * 84) : nullptr;
        catalog = at<Dword>(descriptor, 60);
        std::strcpy(name, network::packets::vehicle_resource(
                              reinterpret_cast<Dword *>(offset(subsystem(52), 4)), int(catalog)));
    }
    at<Dword>(car, 1772) = slot;
    pending::load_car_assets(car, name, options, flags);
    if (!at<Dword>(subsystem(108), 44) || !at<Dword>(subsystem(108), 48))
        at<Dword>(car, 3372) =
            pending::bind_vehicle_slot_entry(subsystem(52), catalog, variant);
    pending::finish_car_initialization(car);
    return car;
}
} // namespace nfs::game::world
