#pragma once
#include "nfs/network/session/network_runtime.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
namespace nfs::network::cipher {
using Byte = std::uint8_t;
struct State {
    const Dword *methods;
    Dword schedule_count;
    Dword *schedule;
    Dword table_blocks;
    Dword *tables;
};
struct Codec {
    const Dword *methods;
    State encrypt, decrypt;
    Dword connection;
    Codec *next;
};
static_assert(sizeof(State) == 20 && sizeof(Codec) == 52 && offsetof(Codec, connection) == 44);
extern const Byte substitutions[2][256];
extern const Dword matrix_tables[4][256], crc_table[256];
extern const Dword encrypt_methods[], decrypt_methods[], state_base_methods[], codec_methods[],
    codec_base_methods[];
extern const Dword state_construct_methods[], state_initial_methods[], codec_abstract_methods[];
Dword rs_remainder(Dword, Dword);
Dword key_word(Dword, const Dword *, Dword);
State *construct_state(State *, const void *, Dword);
void encrypt_block(State *, const Dword *, Dword *);
void decrypt_block(State *, const Dword *, Dword *);
void destroy_encrypt_state(State *);
void destroy_decrypt_state(State *);
void destroy_initial_state(State *);
void destroy_state_base(State *);
Codec *construct_codec(Codec *, const void *, Dword, Dword);
void destroy_codec(Codec *);
Codec *find(Dword);
bool ensure(Dword, Dword);
Dword *checksum(Dword *, const Byte *, Dword);
Byte *encode(Dword, const Byte *, Dword, Dword *);
Byte *decode(Dword, const Byte *, Dword, Dword *);
void *encode_blocks(Codec *, const void *, Dword, Dword *);
void *decode_blocks(Codec *, const void *, Dword, Dword *);
void free_blocks(Codec *, void *);
void *allocate_output(Codec *, Dword);
void release_output(Codec *, void *);
void initialize(lifecycle::System *);
bool register_connection(lifecycle::System *, Dword);
Byte *encode_locked(Dword, const Byte *, Dword, Dword *);
Byte *decode_locked(Dword, const Byte *, Dword, Dword *);
void remove_connection(Dword);
void release_locked(void *);
void *__fastcall codec_delete(Codec *, void *, unsigned);
void *__fastcall encrypt_delete(State *, void *, unsigned);
void *__fastcall decrypt_delete(State *, void *, unsigned);
void *__fastcall state_base_delete(State *, void *, unsigned);
void __fastcall encrypt_in_place(State *, void *, Dword *);
void __fastcall decrypt_in_place(State *, void *, Dword *);
Dword __fastcall block_bytes(State *, void *);
void __fastcall ignore_error(Codec *, void *, const char *, Dword, Dword);
} // namespace nfs::network::cipher
