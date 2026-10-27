#include "nfs/physics/frame/dynamics_runtime.hpp"
#include <initializer_list>
namespace nfs::sound {
// Bytes6..7 are not written by the initializer.
int initialize_parameters(VoiceParameters *parameters) {
    for (unsigned offset : {12u, 14u, 16u})
        field<std::uint16_t>(parameters, offset) = 4096;
    field<Byte>(parameters, 2) = 60;
    field<std::uint16_t>(parameters, 18) = 0;
    field<std::uint16_t>(parameters, 20) = 65535;
    field<std::uint16_t>(parameters, 22) = 0;
    field<Byte>(parameters, 3) = field<Byte>(parameters, 0) = field<Byte>(parameters, 4) =
        field<Byte>(parameters, 5) = 127;
    field<Byte>(parameters, 1) = 64;
    field<std::uint16_t>(parameters, 8) = field<std::uint16_t>(parameters, 10) = 0;
    return 0;
}
} // namespace nfs::sound
