#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "../../math/precision/retained_particle.hpp"
namespace nfs::game::runtime {
namespace {
// Capture the multiplier as a DWORD before the random provider runs.
// A floating local would quiet an SNaN before its original arithmetic use.
__declspec(noinline) Dword captured_period_word(const void *value) {
    return *static_cast<const volatile Dword *>(value);
}
}
// Period multiplier is the first LOD world-particle float.
long double retained_emitter_period(Dword *settings) {
    const Dword period =
        captured_period_word(reinterpret_cast<const void *>(lifecycle::runtime::particle_emitters));
    const Dword random = network::rules::random();
    return (static_cast<long double>(random & 65535) * 0.0000152587890625f *
                at<const volatile float>(settings, 36) +
            at<const volatile float>(settings, 28)) *
           at<const float>(&period, 0);
}
double compatibility_emitter_period(Dword *settings) {
    return double(retained_emitter_period(settings));
}
namespace pending {

void reset_particle_emitter(Dword *object) {
    for (Dword index = 0; static_cast<int>(index) < at<int>(object, 104); ++index) {
        Dword *record = reinterpret_cast<Dword *>(bytes(object) + 108 + index * 24);
        Dword *settings = at<Dword *>(record, 20);
        at<volatile float>(record, 4) = float(retained_emitter_period(settings));
        at<volatile Dword>(record, 0) = 0;
        const Dword random = network::rules::random();
        const long double fraction = static_cast<long double>(random & 65535);
        at<volatile Dword>(record, 8) = 0;
        at<volatile Byte>(record, 16) = 1;
        at<volatile float>(record, 12) =
            float(fraction * 0.0000152587890625f * at<const volatile float>(settings, 12) +
                  at<const volatile float>(settings, 4));
    }
    at<volatile Byte>(object, 232) = 0;
}
} // namespace pending
} // namespace nfs::game::runtime
