#include "comm_connection_runtime.hpp"
namespace nfs::network::session::pending {

void __cdecl comm_worker_entry(Dword address) {
    void *owner = reinterpret_cast<void *>(address);
    while (at<volatile Dword>(owner, 20) != 1) {
        if (at<volatile Dword>(owner, 20) == 2) {
            at<volatile Dword>(owner, 20) = 3;
            return;
        }
        kernel::thread_yield(10);
    }
    while (at<volatile Dword>(owner, 20) != 2)
        if (auto callback = at<void(__cdecl *)(void *)>(owner, 24))
            callback(at<void *>(owner, 28));
    at<volatile Dword>(owner, 20) = 3;
}

void *construct_comm_worker(void *owner, void(__cdecl *callback)(void *), void *context,
                            Dword stack_size, Dword priority) {
    at<void *>(owner, 16) = nullptr;
    at<Dword>(owner, 20) = 0;
    at<void(__cdecl *)(void *)>(owner, 24) = callback;
    at<void *>(owner, 28) = context;
    at<const Dword *>(owner, 0) = comm_worker_methods;
    void *stack = comm::allocate_named("thread", stack_size, 0);
    at<void *>(owner, 16) = stack;
    kernel::thread_start(&at<kernel::Thread>(owner, 4), comm_worker_entry,
                         reinterpret_cast<Dword>(owner), reinterpret_cast<Dword>(stack), stack_size,
                         int(priority));
    at<volatile Dword>(owner, 20) = 1;
    return owner;
}

static void resume_comm_worker(void *owner) {
    at<volatile Dword>(owner, 20) = 1;
}

static Dword stop_comm_worker(void *owner) {
    if (at<volatile Dword>(owner, 20) == 1) {
        at<volatile Dword>(owner, 20) = 2;
        int remaining = 2000;
        while (at<volatile Dword>(owner, 20) != 3) {
            kernel::thread_yield(10);
            remaining -= 10;
            if (remaining < 1)
                return 0;
        }
    }
    return 1;
}

static void destroy_comm_worker(void *owner) {
    at<const Dword *>(owner, 0) = comm_worker_methods;
    stop_comm_worker(owner);
    if (void *stack = at<void *>(owner, 16))
        kernel::memory_free(stack);
}

static void *__fastcall delete_comm_worker(void *owner, void *, Dword flags) {
    destroy_comm_worker(owner);
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}

void __cdecl update_driver_worker(void *driver) {
    if (!driver)
        _assert("pDriver", "cmn\\driver.cpp", 424);
    method(driver, 3, Dword(0));
    kernel::thread_yield(10);
}

Dword start_driver_worker(void *driver, void(__cdecl *callback)(void *), void *context,
                         Dword stack_size, Dword priority) {
    if (*comm_options() == 1) {
        if (!callback)
            callback = update_driver_worker;
        void *storage = gui::application_allocate(32);
        void *worker = storage
                           ? construct_comm_worker(storage, callback, context, stack_size, priority)
                           : nullptr;
        at<void *>(driver, 24) = worker;
        resume_comm_worker(worker);
    }
    return 1;
}
const Dword comm_worker_methods[]{reinterpret_cast<Dword>(delete_comm_worker)};
} // namespace nfs::network::session::pending
