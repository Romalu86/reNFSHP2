#include "nfs/network/transport/network_mailbox.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::network::comm {
Dword last_error = 0;
Dword sequence = 0;

void initialize_kind(Dword *object, Dword kind) {
    *object = kind;
}

void *allocate_named(const char *name, Dword bytes, Dword reverse) {
    return kernel::memory_allocate(name, static_cast<int>(bytes),
                                   static_cast<std::uint8_t>(reverse) ? 256 : 0);
}

void *allocate_object(Dword bytes) {
    return allocate_named("COMM_BASE_OBJECT", bytes, 0);
}
// The confirmed memory-fill dispatch is shared with kernel.
void fill(void *data, Dword value, Dword bytes) {
    kernel::memory_fill(data, value, bytes);
}

void set_error(Dword error) {
    last_error = error;
}

void *__fastcall delete_lock(LockObject *object, void *, unsigned flags) {
    object->methods = lock_methods;
    kernel::lock_destroy(&object->lock);
    if (flags & 1)
        kernel::memory_free(object);
    return object;
}
const gui::Destroy lock_methods[]{reinterpret_cast<gui::Destroy>(delete_lock)};
} // namespace nfs::network::comm
