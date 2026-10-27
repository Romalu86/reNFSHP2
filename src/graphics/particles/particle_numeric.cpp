#include "nfs/physics/frame/simulation_runtime.hpp"
#include <cmath>
#include "nfs/network/session/network_rules.hpp"
#include "../../math/precision/retained_particle.hpp"
namespace nfs::game::runtime {
long double sample_emitter_period(Dword *settings) {
    return compatibility_emitter_period(settings);
}

Dword (&emitter_random_state)[6] = network::rules::random_state;
Dword emitter_random_word() {
    return network::rules::random();
}
// kept external so the leaf providers consume a DWORD constant.
extern const float turn_radians = 6.283185482025146484375f;
// Reread angle after the first write, preserving overlap.
void turn_sine_cosine(const float *angle, float *sine, float *cosine) {
    *sine = float(turn_sine(*angle));
    *cosine = float(turn_cosine(*angle));
}
} // namespace nfs::game::runtime
