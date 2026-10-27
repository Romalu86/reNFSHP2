#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::lifecycle::runtime {
namespace {
Dword reflect(Dword value, Dword bits) {
    Dword source = value;
    for (Dword bit = bits - 1; bits; --bits, --bit, source >>= 1) {
        Dword mask = 1u << (bit & 31);
        if (source & 1)
            value |= mask;
        else
            value &= ~mask;
    }
    return value;
}
} // namespace
// Complete table builder, including reflection and reset.
void build_crc_table(Dword *object) {
    volatile Dword *words = object;
    words[1032 / 4] = words[1028 / 4] == 16 ? 65535 : 4294967295u;
    for (Dword byte = 0; byte < 256; ++byte) {
        const Dword bits = words[1028 / 4];
        const Dword top_bit = 1u << ((bits - 1) & 31);
        Dword value = words[1044 / 4] ? reflect(byte, 8) : byte;
        value <<= (bits - 8) & 31;
        for (int bit = 0; bit < 8; ++bit) {
            if (value & top_bit)
                value = (value << 1) ^ words[1036 / 4];
            else
                value <<= 1;
        }
        if (words[1044 / 4])
            value = reflect(value, bits);
        words[byte + 1] = value & words[1032 / 4];
    }
    words[1052 / 4] = words[1040 / 4];
}
} // namespace nfs::lifecycle::runtime
namespace nfs::lifecycle::table_entries {
void __fastcall initialize_crc(System *object, void *) {
    runtime::build_crc_table(reinterpret_cast<Dword *>(object));
}
} // namespace nfs::lifecycle::table_entries
