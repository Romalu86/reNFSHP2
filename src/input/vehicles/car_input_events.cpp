#include "car_input_events.hpp"
#include "nfs/core/lifecycle/object_state.hpp"
namespace nfs::game::runtime::events {

void deliver(Dword *listener, void *event) {
    if (listener[2] == 100) {
        invoke(listener, 1, event);
        return;
    }
    listener[listener[2] + 3] = reinterpret_cast<Dword>(event);
    acquire(reinterpret_cast<Dword *>(bytes(event) + 28));
    ++listener[2];
}

void enqueue(Dword *manager, const void *event) {
    const Dword kind = at<Dword>(event, 4);
    auto **listeners = reinterpret_cast<Dword **>(bytes(manager) + 3608 + kind * 100);
    Byte *slot = bytes(manager) + 8 + manager[1] * 36;
    for (unsigned occupied = 0;;) {
        if (!at<Dword>(slot, 32)) {
            for (unsigned offset = 0; offset < 20; offset += 4)
                std::memcpy(slot + 8 + offset, bytes(event) + 8 + offset, 4);
            at<Dword>(slot, 4) = kind;
            const int count = at<int>(manager, 4108 + kind * 4);
            for (int index = 0; index < count; ++index)
                deliver(listeners[index], slot);
            return;
        }
        ++manager[1];
        if (manager[1] == 100) {
            manager[1] = 0;
            slot = bytes(manager) + 8;
        } else
            slot += 36;
        if (++occupied == 100)
            return;
    }
}
} // namespace nfs::game::runtime::events
namespace nfs::services::input_events {
using namespace nfs::game::runtime;
// Only nine payload bytes are written in the saved frame.
void emit_action(void *manager, Dword action, Dword value, Byte analog) {
    alignas(4) Byte event[28], payload[20];
    at<const Dword *>(event, 0) =
        nfs::lifecycle::construction::objects::pending::input_event_base_methods;
    at<Dword>(event, 4) = 4;
    kernel::memory_zero(event + 8, 20);
    at<Dword>(payload, 0) = action;
    at<Dword>(payload, 4) = value;
    payload[8] = analog;
    // Transport the unwritten representation without interpreting it as C++ integers.
    for (unsigned offset = 0; offset < 20; offset += 4)
        std::memcpy(event + 8 + offset, payload + offset, 4);
    events::enqueue(static_cast<Dword *>(manager), event);
}

void emit_car_actions(const Dword *controls) {
    const Dword flags = controls[0];
    void *manager = system(60);
    emit_action(manager, 2, Dword(at<std::int8_t>(controls, 4)), Byte((flags >> 15) & 4294967041u));
    emit_action(manager, 0, at<Byte>(controls, 5), Byte((flags >> 13) & 4294967041u));
    emit_action(manager, 1, at<Byte>(controls, 6), Byte((flags >> 14) & 4294967041u));
    static constexpr unsigned bits[] = {2, 1, 5, 3, 4, 6, 7, 8, 9, 10, 11, 12};
    for (unsigned action = 3; action <= 14; ++action)
        emit_action(manager, action, (flags >> bits[action - 3]) & 1, 0);
    emit_action(manager, 15, at<Byte>(controls, 2) & 1, 0);
    for (unsigned action = 16; action <= 22; ++action)
        emit_action(manager, action, (flags >> (action + 1)) & 1, 0);
}

void dispatch_car_actions(void *listener, void *entity, const Dword *controls) {
    void *car = at<void *>(entity, 368);
    const Byte gear = at<Byte>(car, 776);
    at<void *>(listener, 416) = car;
    at<Byte>(listener, 420) = gear;
    emit_car_actions(controls);
    events::dispatch(static_cast<Dword *>(listener));
    at<Dword>(listener, 416) = 0;
}
} // namespace nfs::services::input_events
