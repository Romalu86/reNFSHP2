#include "nfs/network/packets/network_cipher.hpp"
#include <cstring>
namespace nfs::network::cipher {
namespace {
void copy_forward(void *destination, const void *source, Dword count) {
    auto *out = static_cast<Byte *>(destination);
    auto *in = static_cast<const Byte *>(source);
    Dword offset = 0;
    for (; offset < (count & ~3u); offset += 4)
        field<Dword>(out, offset) = field<Dword>(const_cast<Byte *>(in), offset);
    for (; offset < count; ++offset)
        out[offset] = in[offset];
}
template <class R, class... A> R call(Codec *codec, unsigned slot, A... args) {
    using Function = R(__thiscall *)(Codec *, A...);
    return reinterpret_cast<Function>(codec->methods[slot])(codec, args...);
}
} // namespace

Codec *find(Dword connection) {
    for (auto *node = reinterpret_cast<Codec *>(lifecycle::runtime::crypt_head); node;
         node = node->next)
        if (node->connection == connection)
            return node;
    return nullptr;
}

bool ensure(Dword key, Dword connection) {
    if (find(connection))
        return true;
    auto *codec = static_cast<Codec *>(gui::application_allocate(sizeof(Codec)));
    if (!codec)
        return false;
    try {
        construct_codec(codec, &key, 4, connection);
    } catch (...) {
        gui::application_release(codec);
        throw;
    }
    return true;
}
// Rounded blocks deliberately read the saved padded extent.
void *encode_blocks(Codec *codec, const void *data, Dword bytes, Dword *written) {
    if (!data)
        return nullptr;
    try {
        *written = ((bytes >> 4) + Dword((bytes & 15) != 0)) * 16;
        void *result = call<void *>(codec, 2, *written);
        if (result)
            for (Dword index = 0; index < (*written >> 4); ++index) {
                using Transform = void(__thiscall *)(State *, const void *, void *);
                reinterpret_cast<Transform>(codec->encrypt.methods[1])(
                    &codec->encrypt, static_cast<const Byte *>(data) + index * 16,
                    static_cast<Byte *>(result) + index * 16);
            }
        return result;
    } catch (...) {
        call<void>(codec, 1, "Crypt::Encrypt Unhandled Exception", Dword(0), Dword(0));
        return nullptr;
    }
}

void *decode_blocks(Codec *codec, const void *data, Dword bytes, Dword *written) {
    if (!data)
        return nullptr;
    try {
        *written = ((bytes >> 4) + Dword((bytes & 15) != 0)) * 16;
        void *result = call<void *>(codec, 2, *written);
        if (result)
            for (Dword index = 0; index < (*written >> 4); ++index) {
                using Transform = void(__thiscall *)(State *, const void *, void *);
                reinterpret_cast<Transform>(codec->decrypt.methods[1])(
                    &codec->decrypt, static_cast<const Byte *>(data) + index * 16,
                    static_cast<Byte *>(result) + index * 16);
            }
        return result;
    } catch (...) {
        call<void>(codec, 1, "Crypt::Decrypt Unhandled Exception", Dword(0), Dword(0));
        return nullptr;
    }
}

void free_blocks(Codec *codec, void *memory) {
    call<void>(codec, 3, memory);
}

Byte *encode(Dword connection, const Byte *data, Dword bytes, Dword *written) {
    *written = 0;
    if (!data)
        return nullptr;
    Codec *codec = find(connection);
    if (!codec)
        return nullptr;
    Dword crc = 0, source_bytes = 0;
    --bytes;
    if (bytes) {
        checksum(&crc, data + 1, bytes);
        source_bytes = bytes;
        void *encrypted = call<void *>(codec, 4, data + 1, bytes, written);
        if (encrypted && *written) {
            const Dword count = *written;
            auto *packet = call<Byte *>(codec, 2, count + 12);
            const Byte opcode = data[0] | 128;
            field<Dword>(packet, 4) = crc;
            packet[0] = opcode;
            field<Dword>(packet, 8) = bytes;
            copy_forward(packet + 12, encrypted, count);
            free_blocks(codec, encrypted);
            *written = count + 12;
            return packet;
        }
    }
    auto *packet = call<Byte *>(codec, 2, Dword(12));
    packet[0] = data[0] | 128;
    field<Dword>(packet, 4) = crc;
    field<Dword>(packet, 8) = source_bytes;
    *written = 12;
    return packet;
}

Byte *decode(Dword connection, const Byte *data, Dword bytes, Dword *written) {
    *written = 0;
    if (!data || bytes <= 11 || bytes > 65536 || field<Dword>(const_cast<Byte *>(data), 8) > bytes)
        return nullptr;
    Codec *codec = find(connection);
    if (!codec || !(data[0] & 128))
        return nullptr;
    if (static_cast<int>(bytes - 12) > 0) {
        void *plain = call<void *>(codec, 5, data + 12, bytes - 12, written);
        if (plain) {
            const Dword source_bytes = field<Dword>(const_cast<Byte *>(data), 8);
            Dword crc;
            checksum(&crc, static_cast<Byte *>(plain), source_bytes);
            if (crc != field<Dword>(const_cast<Byte *>(data), 4)) {
                call<void>(codec, 3, plain);
                call<void>(codec, 1, "Error, CRC missmatch in data stream", Dword(0), Dword(0));
                return nullptr;
            }
            const Dword count = *written;
            auto *packet = call<Byte *>(codec, 2, count + 1);
            packet[0] = data[0] & 127;
            copy_forward(packet + 1, plain, count);
            free_blocks(codec, plain);
            *written = source_bytes + 1;
            return packet;
        }
    }
    auto *packet = call<Byte *>(codec, 2, Dword(1));
    packet[0] = data[0] & 127;
    *written = 1;
    return packet;
}
} // namespace nfs::network::cipher
