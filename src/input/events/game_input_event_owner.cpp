#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "game_input_event_runtime.hpp"
#include "../../network/vehicles/remote_car_runtime.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
#include "nfs/core/errors/assertions.hpp"
#include <cstdlib>
namespace nfs::services::pending {
alignas(4) Byte input_event[424]{};
}
namespace nfs::services::input_events {
using game::runtime::at;
using game::runtime::invoke;
namespace {
extern const Dword listener_methods[2], base_listener_methods[2];
Byte destroyed{};
} // namespace

void *construct_event_listener(void *listener) {
    nfs::lifecycle::construction::objects::event_member(listener);
    try {
        at<const Dword *>(listener, 0) = base_listener_methods;
        game::runtime::events::reset_references(static_cast<Dword *>(listener));
        at<Dword>(listener, 8) = 0;
        kernel::memory_zero(static_cast<Byte *>(listener) + 12, 400);
    } catch (...) {
        nfs::lifecycle::construction::objects::event_member_destroy(listener);
        throw;
    }
    return listener;
}

void destroy_event_listener(void *listener) {
    at<const Dword *>(listener, 0) = base_listener_methods;
    nfs::lifecycle::construction::objects::event_member_destroy(listener);
}

void *__fastcall delete_event_listener(void *listener, void *, Byte flags) {
    destroy_event_listener(listener);
    if (flags & 1)
        gui::application_release(listener);
    return listener;
}

void *construct_input_listener(void *listener) {
    construct_event_listener(listener);
    at<Byte>(listener, 412) = 0;
    at<Dword>(listener, 416) = 0;
    at<Byte>(listener, 420) = 1;
    at<const Dword *>(listener, 0) = listener_methods;
    return listener;
}

void destroy_input_listener(void *listener) {
    at<const Dword *>(listener, 0) = listener_methods;
    destroy_event_listener(listener);
}

void *__fastcall delete_input_listener(void *listener, void *, Byte flags) {
    destroy_input_listener(listener);
    if (flags & 1)
        gui::application_release(listener);
    return listener;
}

void __fastcall receive_vehicle_input(void *listener, void *, void *event) {
    using namespace game::world;
    if (at<Dword>(event, 4) != 4)
        return;
    void *car = at<void *>(listener, 416);
    if (!car)
        return;
    const int value = at<int>(event, 12);
    const Dword action = at<Dword>(event, 8);
    auto accepts_camera_input = [&]() {
        const int player = at<int>(car, 1776);
        if (player >= 0 && player < 2)
            return true;
        return network::session_active() && at<std::int16_t>(subsystem(104), 10) >= 4 &&
               at<void *>(game::tick_services[0], 300) == car;
    };
    auto player_camera = [&]() -> void * {
        const int player = at<int>(car, 1776);
        return game::tick_services[player >= 0 && player < 2 ? player : 0];
    };
    switch (action) {
    case 0:
        at<Byte>(car, 1576) = Byte(value);
        at<Byte>(car, 1592) = at<Byte>(event, 16);
        return;
    case 1:
        at<Byte>(car, 1581) = Byte(value);
        at<Byte>(car, 1593) = at<Byte>(event, 16);
        return;
    case 2:
        at<Byte>(car, 1594) = at<Byte>(event, 16);
        store_steering_input(&at<float>(car, 1600), value);
        return;
    case 3: {
        if (!accepts_camera_input())
            return;
        if (at<Byte>(listener, 412)) {
            if (value) {
                at<Byte>(listener, 412) = 0;
                cycle_camera(player_camera());
                return;
            }
        } else if (value)
            return;
        at<Byte>(listener, 412) = 1;
        return;
    }
    case 4:
        at<Byte>(car, 1585) = Byte(value);
        return;
    case 5:
        if (value) {
            void *entity = at<void *>(car, 7988);
            void *perpetrator = entity ? invoke<void *>(entity, 11) : nullptr;
            if (perpetrator) {
                if (!reset_allowed(perpetrator)) {
                    at<Byte>(car, 1816) = 1;
                    return;
                }
                float speed;
                store_reset_speed(&speed, at<float>(subsystem(76), 84420));
                game::runtime::physics::request_car_reset(car, speed);
            } else
                game::runtime::physics::request_car_reset(car, 15.64605045318603515625f);
        }
        return;
    case 6:
        if (value && !at<Byte>(listener, 420))
            at<Byte>(car, 1582) = Byte(game::runtime::physics::contact::request_next_gear(car));
        return;
    case 7:
        if (value && !at<Byte>(listener, 420))
            at<Byte>(car, 1582) = Byte(request_previous_gear(car));
        return;
    case 8:
        if (accepts_camera_input())
            at<Byte>(player_camera(), 164) = value != 0;
        return;
    case 11:
        at<Byte>(car, 1577) = Byte(value);
        return;
    default:
        if (action >= 15 && action <= 22 && value && !at<Byte>(listener, 420))
            at<Byte>(car, 1582) = Byte(request_gear(car, int(action - 15)));
    }
}
namespace {
const Dword base_listener_methods[2] = {reinterpret_cast<Dword>(delete_event_listener),
                                       reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call)};
const Dword listener_methods[2] = {reinterpret_cast<Dword>(delete_input_listener),
                                  reinterpret_cast<Dword>(receive_vehicle_input)};
} // namespace

void release_input_listener() {
    if (!(destroyed & 1)) {
        destroyed |= 1;
        destroy_input_listener(services::pending::input_event);
    }
}
 
void initialize_input_listener() {
    construct_input_listener(services::pending::input_event);
    std::atexit(release_input_listener);
}
} // namespace nfs::services::input_events
NFS_KERNEL_INITIALIZER(startup_initialize_input_listener, ".CRT$XCU_NFS_006902A8",
                       nfs::services::input_events::initialize_input_listener);
