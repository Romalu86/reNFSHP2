#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include <cstring>
namespace nfs::lifecycle::table_entries {
using namespace game::runtime;
extern const char replay_name[];

void __fastcall reset_replay_mode(System *object, void *) {
    const Dword replay = at<volatile Dword>(system(72), 140);
    at<volatile Dword>(object, 40) = 0;
    at<volatile Dword>(object, 44) = 0;
    std::strncpy(reinterpret_cast<char *>(object) + 8, replay_name, 32);
    if (replay) {
        void *header = at<void *>(object, 4);
        const Dword mode = at<volatile Byte>(header, 8);
        at<volatile Dword>(object, 56) = at<volatile Dword>(header, 0) & 1;
        const Dword flags = at<volatile Dword>(header, 16);
        at<volatile Dword>(object, 64) = mode;
        const Dword vehicle = at<volatile Byte>(header, 24);
        at<volatile Byte>(object, 39) = 0;
        at<volatile Dword>(object, 52) = 0;
        at<volatile Dword>(object, 68) = vehicle;
        at<volatile Dword>(object, 60) = flags & 1;
    } else {
        at<volatile Byte>(object, 39) = 0;
        at<volatile Dword>(object, 52) = 0;
    }
}
} // namespace nfs::lifecycle::table_entries
