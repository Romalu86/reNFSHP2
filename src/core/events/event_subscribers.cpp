#include "nfs/core/lifecycle/subsystem_runtime.hpp"
namespace nfs::game::runtime::events {
// Thiscall carries one unused stack argument, removed at C++ callers.
void acquire(Dword *object) {
    ++object[1];
}

void release(Dword *object) {
    --object[1];
}

void reset_references(Dword *object) {
    object[1] = 0;
}

void clear(Dword *object) {
    reset_references(object);
    object[2] = 0;
    kernel::memory_zero(object + 3, 400);
}

void remove_at(Dword *object, int index) {
    if (static_cast<int>(object[2]) > 0 && index >= 0 && index < static_cast<int>(object[2])) {
        if (index < static_cast<int>(object[2]) - 1)
            do {
                object[3 + index] = object[4 + index];
                ++index;
            } while (index < static_cast<int>(object[2]) - 1);
        --object[2];
    }
}

void dispatch(Dword *object) {
    for (int index = 0; index < static_cast<int>(object[2]); ++index) {
        void *event = reinterpret_cast<void *>(object[index + 3]);
        release(reinterpret_cast<Dword *>(bytes(event) + 28));
        invoke(object, 1, event);
    }
    object[2] = 0;
}

void subscribe(Dword *manager, int kind, Dword *listener) {
    int &count = at<int>(manager, 4108 + kind * 4);
    if (count != 25) {
        at<Dword *>(manager, 3608 + (kind * 25 + count) * 4) = listener;
        ++count;
        acquire(listener);
    }
}

void unsubscribe_kind(Dword *manager, int kind, Dword *listener) {
    auto **list = reinterpret_cast<Dword **>(bytes(manager) + 3608 + kind * 100);
    for (int index = 0; index < 25; ++index)
        if (list[index] == listener) {
            release(listener);
            int count = int(listener[2]);
            for (int item = 0; item < count; ++item) {
                auto *event = reinterpret_cast<Dword *>(listener[3 + item]);
                if (event[1] == Dword(kind)) {
                    release(event + 7);
                    remove_at(listener, item);
                    --item;
                    --count;
                }
            }
            int &registered = at<int>(manager, 4108 + kind * 4);
            if (registered > 0) {
                list[index] = list[registered - 1];
                --registered;
            }
            return;
        }
}

void unsubscribe(Dword *manager, Dword *listener) {
    for (int kind = 0; kind < 5; ++kind) {
        auto **listeners = reinterpret_cast<Dword **>(bytes(manager) + 3608 + kind * 100);
        int index = 0;
        while (index < 25 && listeners[index] != listener)
            ++index;
        if (index == 25)
            continue;
        if (static_cast<int>(listener[1]) > 0)
            release(listener);
        int count = static_cast<int>(listener[2]);
        for (int item = 0; item < count; ++item) {
            auto *event = reinterpret_cast<Dword *>(listener[3 + item]);
            if (event[1] == Dword(kind)) {
                if (static_cast<int>(event[8]) > 0)
                    release(event + 7);
                remove_at(listener, item);
                --item;
                --count;
            }
        }
        int &registered = at<int>(manager, 4108 + kind * 4);
        if (registered > 0) {
            listeners[index] = listeners[registered - 1];
            --registered;
        }
    }
}
void clear_subscribers(Dword *manager) {
    for (int kind = 0; kind < 5; ++kind) {
        auto **list = reinterpret_cast<Dword **>(bytes(manager) + 3608 + kind * 100);
        for (int left = at<int>(manager, 4108 + kind * 4); left > 0; --left, ++list)
            clear(*list);
    }
}
} // namespace nfs::game::runtime::events
