#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::game::runtime::physics::pending {
// Preserve padding and fields not assigned by the reset routine.
void reset_vehicle_entry(void *state) {
    at<Dword>(state, 0) = 0;
    at<Byte>(state, 4) = 0;
    at<Byte>(state, 728) = 0;
    at<Byte>(state, 729) = 0;
    at<Byte>(state, 5) = 0;
    at<Dword>(state, 676) = 0;
    at<Byte>(state, 730) = 0;
    kernel::memory_zero(bytes(state) + 680, 12);
    kernel::memory_zero(bytes(state) + 692, 36);
    kernel::memory_zero(bytes(state) + 8, 12);
    kernel::memory_zero(bytes(state) + 20, 12);
    at<Dword>(state, 32) = 0;
    at<Dword>(state, 36) = 0;
    at<Dword>(state, 40) = 31;
    kernel::memory_zero(bytes(state) + 44, 12);
    at<Dword>(state, 56) = at<Dword>(state, 60) = at<Dword>(state, 64) = at<Dword>(state, 68) =
        0;
    kernel::memory_zero(bytes(state) + 72, 36);
    kernel::memory_zero(bytes(state) + 108, 12);
    kernel::memory_zero(bytes(state) + 120, 12);
    at<Dword>(state, 132) = at<Dword>(state, 136) = 0;
    kernel::memory_zero(bytes(state) + 140, 312);
    kernel::memory_zero(bytes(state) + 452, 12);
    at<Dword>(state, 464) = 0;
    kernel::memory_zero(bytes(state) + 468, 12);
    at<Dword>(state, 480) = at<Dword>(state, 484) = at<Dword>(state, 488) = 0;
    kernel::memory_zero(bytes(state) + 492, 12);
    at<Dword>(state, 504) = 0;
    kernel::memory_zero(bytes(state) + 508, 84);
    at<Dword>(state, 592) = 0;
    at<std::uint16_t>(state, 596) = 0;
    at<Dword>(state, 600) = 0;
    kernel::memory_zero(bytes(state) + 604, 72);
    at<Dword>(state, 732) = 0;
}
} // namespace nfs::game::runtime::physics::pending
