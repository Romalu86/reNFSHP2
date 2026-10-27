#include "nfs/network/packets/network_quantizers.hpp"
#include "../../math/precision/extended_float.hpp"
#include "network_packet_x87.hpp"
#include <cstring>
namespace nfs::network::bits {
// Preserve name tail/padding, as in the constructor.
Quantizer *construct_quantizer(Quantizer *object) {
    for (unsigned offset = 300; offset <= 324; offset += 4)
        field<Dword>(object, offset) = 0;
    object->bytes[328] = 0;
    object->bytes[0] = 0;
    return object;
}
// Exact single-byte RET body.
void destroy_quantizer(Quantizer *) {
}

Quantizer *configure_bits(Quantizer *object, const char *name, float minimum, float maximum, Dword width) {
    return packet_native::configure_quantizer(object, name, minimum, maximum, width);
}
} // namespace nfs::network::bits
