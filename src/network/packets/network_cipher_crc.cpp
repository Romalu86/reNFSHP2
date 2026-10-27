#include "nfs/network/packets/network_cipher.hpp"
namespace nfs::network::cipher {
// The saved CRC has no final complement.
Dword *checksum(Dword *result, const Byte *data, Dword bytes) {
    if (!data) {
        *result = 0;
        return result;
    }
    Dword value = 4294967295u;
    while (bytes--)
        value = (value >> 8) ^ crc_table[(*data++ ^ (value & 255)) & 255];
    *result = value;
    return result;
}
} // namespace nfs::network::cipher
