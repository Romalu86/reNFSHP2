#include "nfs/network/packets/network_cipher.hpp"
#include <cstdlib>
namespace nfs::network::cipher {
namespace {
// Keep the TCP protocol key stable across executable builds for LAN compatibility.
 
 
 
 
constexpr Dword retail_protocol_key = 2579795945u;
} // namespace

void initialize(lifecycle::System *object) {
    auto *fields = reinterpret_cast<Dword *>(object);
    fields[1] = retail_protocol_key;
    InitializeCriticalSection(&lifecycle::runtime::crypt_lock);
}

bool register_connection(lifecycle::System *object, Dword connection) {
    EnterCriticalSection(&lifecycle::runtime::crypt_lock);
    const bool result = ensure(reinterpret_cast<Dword *>(object)[1], connection);
    LeaveCriticalSection(&lifecycle::runtime::crypt_lock);
    return result;
}

Byte *encode_locked(Dword connection, const Byte *data, Dword bytes, Dword *written) {
    EnterCriticalSection(&lifecycle::runtime::crypt_lock);
    Byte *result = encode(connection, data, bytes, written);
    LeaveCriticalSection(&lifecycle::runtime::crypt_lock);
    return result;
}

Byte *decode_locked(Dword connection, const Byte *data, Dword bytes, Dword *written) {
    EnterCriticalSection(&lifecycle::runtime::crypt_lock);
    Byte *result = decode(connection, data, bytes, written);
    LeaveCriticalSection(&lifecycle::runtime::crypt_lock);
    return result;
}

void remove_connection(Dword connection) {
    EnterCriticalSection(&lifecycle::runtime::crypt_lock);
    if (Codec *codec = find(connection)) {
        using Delete = void(__thiscall *)(Codec *, Dword);
        reinterpret_cast<Delete>(codec->methods[0])(codec, 1);
    }
    LeaveCriticalSection(&lifecycle::runtime::crypt_lock);
}

void release_locked(void *memory) {
    EnterCriticalSection(&lifecycle::runtime::crypt_lock);
    std::free(memory);
    LeaveCriticalSection(&lifecycle::runtime::crypt_lock);
}
} // namespace nfs::network::cipher
