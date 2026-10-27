#include "nfs/network/transport/thread_queue.hpp"
namespace nfs::lifecycle::construction::network::queue {

void clear(Dword *object) {
    if (object[15])
        EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(object + 9));
    auto *node = reinterpret_cast<Dword *>(object[5]);
    while (node) {
        auto *next = reinterpret_cast<Dword *>(node[1]);
        using Release = void(__thiscall *)(Dword *, Dword);
        reinterpret_cast<const Release *>(object[0])[7](object, node[4]);
        gui::application_release(node);
        node = next;
    }
    object[7] = 0;
    object[16] = 1;
    object[5] = 0;
    object[6] = 0;
    if (object[15])
        LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(object + 9));
}
} // namespace nfs::lifecycle::construction::network::queue
