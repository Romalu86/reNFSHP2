#include "nfs/network/session/network_rules.hpp"
namespace nfs::network::rules {
Dword random_state[6] = {4063039062u, 2284922601u, 3324304687u,
                        117621916, 2654289789u, 1876900708};

Dword random() {
    Dword a = random_state[4] + random_state[5];
    Dword carry = (a < random_state[4] || a < random_state[5]);
    Dword b = a + carry + random_state[3], c = b + (b < random_state[3]) + random_state[2],
         d = c + (c < random_state[2]) + random_state[1];
    random_state[0] += d + (d < random_state[1]);
    random_state[4] = a;
    random_state[3] = b;
    random_state[2] = c;
    random_state[1] = d;
    if (++random_state[5] == 0 && ++random_state[4] == 0 && ++random_state[3] == 0 &&
        ++random_state[2] == 0 && ++random_state[1] == 0)
        ++random_state[0];
    return random_state[0];
}
} // namespace nfs::network::rules
