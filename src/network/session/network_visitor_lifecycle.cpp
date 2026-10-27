#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include "nfs/network/session/network_player_state.hpp"
#include <cstdlib>
namespace nfs::network::players {
namespace base = lifecycle::construction::network::pending;
// Four-byte visitor objects. CRT initializers set up the four phase visitors and the slot
// comparer.
Visitor ready{}, start{}, prepare{}, count{}, slot_comparer{};

Dword selected_players(Dword *queue) {
    Dword total = 0;
    lifecycle::construction::network::queue::visit(queue, 0, reinterpret_cast<Dword>(&total),
                                                   reinterpret_cast<Dword *>(&count), 0, nullptr);
    return total;
}

Visitor *construct_ready(Visitor *object) {
    object->methods = base::visitor_interface_methods;
    object->methods = ready_methods;
    return object;
}

Visitor *construct_start(Visitor *object) {
    object->methods = base::visitor_interface_methods;
    object->methods = start_methods;
    return object;
}

Visitor *construct_prepare(Visitor *object) {
    object->methods = base::visitor_interface_methods;
    object->methods = prepare_methods;
    return object;
}

Visitor *construct_count(Visitor *object) {
    object->methods = base::visitor_interface_methods;
    object->methods = count_methods;
    return object;
}

Visitor *construct_slot(Visitor *object) {
    object->methods = base::comparer_interface_methods;
    object->methods = slot_methods;
    return object;
}

void destroy_ready(Visitor *object) {
    object->methods = base::visitor_interface_methods;
}

void destroy_start(Visitor *object) {
    object->methods = base::visitor_interface_methods;
}

void destroy_prepare(Visitor *object) {
    object->methods = base::visitor_interface_methods;
}

void destroy_count(Visitor *object) {
    object->methods = base::visitor_interface_methods;
}

void destroy_slot(Visitor *object) {
    object->methods = base::comparer_interface_methods;
}

void *__fastcall delete_ready(Visitor *object, void *, unsigned flags) {
    destroy_ready(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

void *__fastcall delete_start(Visitor *object, void *, unsigned flags) {
    destroy_start(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

void *__fastcall delete_prepare(Visitor *object, void *, unsigned flags) {
    destroy_prepare(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

void *__fastcall delete_count(Visitor *object, void *, unsigned flags) {
    destroy_count(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}

void *__fastcall delete_slot(Visitor *object, void *, unsigned flags) {
    destroy_slot(object);
    if (flags & 1)
        gui::application_release(object);
    return object;
}
const Dword ready_methods[]{reinterpret_cast<Dword>(reset_slot),
                           reinterpret_cast<Dword>(delete_ready)};
const Dword start_methods[]{reinterpret_cast<Dword>(start_vehicle),
                           reinterpret_cast<Dword>(delete_start)};
const Dword prepare_methods[]{reinterpret_cast<Dword>(prepare_slot),
                             reinterpret_cast<Dword>(delete_prepare)};
const Dword count_methods[]{reinterpret_cast<Dword>(count_slot),
                           reinterpret_cast<Dword>(delete_count)};
const Dword slot_methods[]{reinterpret_cast<Dword>(compare_slot),
                          reinterpret_cast<Dword>(delete_slot)};

void finalize_ready() {
    destroy_ready(&ready);
}

void initialize_ready() {
    construct_ready(&ready);
    std::atexit(finalize_ready);
}

void finalize_start() {
    destroy_start(&start);
}

void initialize_start() {
    construct_start(&start);
    std::atexit(finalize_start);
}

void finalize_prepare() {
    destroy_prepare(&prepare);
}

void initialize_prepare() {
    construct_prepare(&prepare);
    std::atexit(finalize_prepare);
}

void finalize_count() {
    destroy_count(&count);
}

void initialize_count() {
    construct_count(&count);
    std::atexit(finalize_count);
}

void finalize_slot() {
    destroy_slot(&slot_comparer);
}

void initialize_slot() {
    construct_slot(&slot_comparer);
    std::atexit(finalize_slot);
}
} // namespace nfs::network::players


NFS_KERNEL_INITIALIZER(startup_initialize_start, ".CRT$XCU_NFS_0069024C",
                       nfs::network::players::initialize_start);
NFS_KERNEL_INITIALIZER(startup_initialize_prepare, ".CRT$XCU_NFS_00690250",
                       nfs::network::players::initialize_prepare);
NFS_KERNEL_INITIALIZER(startup_initialize_ready, ".CRT$XCU_NFS_00690254",
                       nfs::network::players::initialize_ready);
NFS_KERNEL_INITIALIZER(startup_initialize_count, ".CRT$XCU_NFS_00690258",
                       nfs::network::players::initialize_count);
