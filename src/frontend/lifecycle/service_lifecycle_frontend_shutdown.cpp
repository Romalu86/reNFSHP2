#include "nfs/core/services/service_lifecycle.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>

namespace nfs::services {

void shutdown_frontend(void *object) {
    if (void *archive = field<void *>(object, 80)) {
        kernel::memory_free(archive);
        field<void *>(object, 80) = nullptr;
    }
    for (unsigned offset : {4u, 8u}) {
        if (void *screen = field<void *>(object, offset)) {
            using Close = void(__thiscall *)(void *);
            (*reinterpret_cast<const Close **>(screen))[248 / 4](screen);
            using Destroy = void *(__thiscall *)(void *, unsigned);
            (*reinterpret_cast<const Destroy **>(screen))[0](screen, 1);
            field<void *>(object, offset) = nullptr;
        }
    }
}
} // namespace nfs::services
