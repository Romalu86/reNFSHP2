#include "nfs/physics/frame/simulation_runtime.hpp"
namespace nfs::game::runtime::ai {

Dword random_state[6] = {4063039062u, 2284922601u, 3324304687u, 117621916, 2654289789u, 1876900708};
Dword random_calls{};

Dword random_word() {
    ++random_calls;
    const Dword low = random_state[4] + random_state[5];
    Dword carry = low < random_state[5] || low < random_state[4];
    Dword value = low + carry + random_state[3];
    carry = value < random_state[3];
    random_state[4] = low;
    Dword next = value + carry + random_state[2];
    carry = next < random_state[2];
    random_state[3] = value;
    value = next + carry + random_state[1];
    carry = value < random_state[1];
    random_state[2] = next;
    random_state[1] = value;
    Dword result = random_state[0] + value + carry;
    ++random_state[5];
    random_state[0] = result;
    if (!random_state[5] && !++random_state[4] && !++random_state[3] && !++random_state[2] &&
        !++random_state[1])
        random_state[0] = ++result;
    return result;
}
} // namespace nfs::game::runtime::ai
