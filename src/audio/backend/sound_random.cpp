#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::sound::pending {
Dword audio_random_state[6]{};
// This six-word BSS generator is distinct from .
Dword next_audio_random_value() {
    Dword *s = audio_random_state;
    Dword a = s[4] + s[5], carry = a < s[4] || a < s[5];
    Dword b = a + carry + s[3], c = b + (b < s[3]) + s[2], d = c + (c < s[2]) + s[1];
    s[0] += d + (d < s[1]);
    s[4] = a;
    s[3] = b;
    s[2] = c;
    s[1] = d;
    if (++s[5] == 0 && ++s[4] == 0 && ++s[3] == 0 && ++s[2] == 0 && ++s[1] == 0)
        ++s[0];
    return s[0];
}
} // namespace nfs::sound::pending
