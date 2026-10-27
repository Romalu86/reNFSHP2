#include "nfs/core/errors/assertions.hpp"
#include "nfs/network/packets/network_cipher.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
namespace nfs::network::cipher {
namespace {
void wipe_state(State *state) {
    bool schedule_pending = true;
    try {
        std::memset(state->tables, 0, (state->table_blocks << 10) & ~Dword(3));
        lifecycle::construction::network::entries::release_payload(state->tables);
        schedule_pending = false;
        std::memset(state->schedule, 0, (state->schedule_count & 1073741823) * 4);
        lifecycle::construction::network::entries::release_payload(state->schedule);
    } catch (...) {
        try {
            if (schedule_pending) {
                std::memset(state->schedule, 0, (state->schedule_count & 1073741823) * 4);
                lifecycle::construction::network::entries::release_payload(state->schedule);
            }
            state->methods = state_base_methods;
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    state->methods = state_base_methods;
}
void destroy_codec_members(Codec *codec) {
    bool decrypt_pending = true;
    try {
        destroy_decrypt_state(&codec->decrypt);
        decrypt_pending = false;
        destroy_encrypt_state(&codec->encrypt);
    } catch (...) {
        if (decrypt_pending)
            try {
                destroy_encrypt_state(&codec->encrypt);
            } catch (...) {
                std::terminate();
            }
        throw;
    }
}
void __fastcall encrypt_entry(State *state, void *, const Dword *in, Dword *out) {
    encrypt_block(state, in, out);
}
void __fastcall decrypt_entry(State *state, void *, const Dword *in, Dword *out) {
    decrypt_block(state, in, out);
}
void *__fastcall allocate_entry(Codec *codec, void *, Dword bytes) {
    return allocate_output(codec, bytes);
}
void __fastcall release_entry(Codec *codec, void *, void *memory) {
    release_output(codec, memory);
}
void *__fastcall encode_entry(Codec *codec, void *, const void *data, Dword bytes, Dword *written) {
    return encode_blocks(codec, data, bytes, written);
}
void *__fastcall decode_entry(Codec *codec, void *, const void *data, Dword bytes, Dword *written) {
    return decode_blocks(codec, data, bytes, written);
}
void __fastcall free_entry(Codec *codec, void *, void *memory) {
    free_blocks(codec, memory);
}
} // namespace

void destroy_encrypt_state(State *state) {
    wipe_state(state);
}

void destroy_decrypt_state(State *state) {
    wipe_state(state);
}

void destroy_initial_state(State *state) {
    wipe_state(state);
}

void destroy_state_base(State *state) {
    state->methods = state_base_methods;
}

void *__fastcall encrypt_delete(State *state, void *, unsigned flags) {
    destroy_encrypt_state(state);
    if (flags & 1)
        gui::application_release(state);
    return state;
}

void *__fastcall decrypt_delete(State *state, void *, unsigned flags) {
    destroy_decrypt_state(state);
    if (flags & 1)
        gui::application_release(state);
    return state;
}

void *__fastcall state_base_delete(State *state, void *, unsigned flags) {
    state->methods = state_base_methods;
    if (flags & 1)
        gui::application_release(state);
    return state;
}

void *__fastcall initial_delete(State *state, void *, unsigned flags) {
    destroy_initial_state(state);
    if (flags & 1)
        gui::application_release(state);
    return state;
}

void *__fastcall initial_base_delete(State *state, void *, unsigned flags) {
    destroy_state_base(state);
    if (flags & 1)
        gui::application_release(state);
    return state;
}

Codec *construct_codec(Codec *codec, const void *key, Dword bytes, Dword connection) {
    construct_state(&codec->encrypt, key, bytes);
    codec->encrypt.methods = encrypt_methods;
    try {
        construct_state(&codec->decrypt, key, bytes);
    } catch (...) {
        try {
            destroy_encrypt_state(&codec->encrypt);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    codec->decrypt.methods = decrypt_methods;
    codec->methods = codec_base_methods;
    codec->connection = connection;
    codec->next = nullptr;
    codec->methods = codec_methods;
    codec->next = reinterpret_cast<Codec *>(lifecycle::runtime::crypt_head);
    lifecycle::runtime::crypt_head = reinterpret_cast<Dword *>(codec);
    return codec;
}

void destroy_codec(Codec *codec) {
    codec->methods = codec_methods;
    Codec *previous = nullptr, *node = reinterpret_cast<Codec *>(lifecycle::runtime::crypt_head);
    while (node) {
        if (node == codec) {
            if (previous)
                previous->next = codec->next;
            else
                lifecycle::runtime::crypt_head = reinterpret_cast<Dword *>(codec->next);
            break;
        }
        previous = node;
        node = node->next;
    }
    codec->methods = codec_base_methods;
    destroy_codec_members(codec);
}

void *__fastcall codec_delete(Codec *codec, void *, unsigned flags) {
    destroy_codec(codec);
    if (flags & 1)
        gui::application_release(codec);
    return codec;
}

void destroy_base_codec(Codec *codec) {
    codec->methods = codec_base_methods;
    destroy_codec_members(codec);
}

void *__fastcall base_codec_delete(Codec *codec, void *, unsigned flags) {
    destroy_base_codec(codec);
    if (flags & 1)
        gui::application_release(codec);
    return codec;
}

void destroy_partial_codec(Codec *codec) {
    codec->methods = codec_base_methods;
    destroy_codec_members(codec);
}

void *__fastcall partial_codec_delete(Codec *codec, void *, unsigned flags) {
    destroy_partial_codec(codec);
    if (flags & 1)
        gui::application_release(codec);
    return codec;
}
// Confirmed RET 0Ch, not an inferred error policy.
void __fastcall ignore_error(Codec *, void *, const char *, Dword, Dword) {
}
// ThrowInfo points to the saved .PAD pointer type.
void __fastcall throw_error(Codec *, void *, const char *message, Dword, Dword) {
    throw const_cast<char *>(message);
}

void *allocate_output(Codec *, Dword bytes) {
    return std::malloc(bytes);
}

void release_output(Codec *, void *memory) {
    std::free(memory);
}

void *__fastcall base_allocate(Codec *, void *, Dword bytes) {
    return std::malloc(bytes);
}

void __fastcall base_release(Codec *, void *, void *memory) {
    std::free(memory);
}
const Dword state_base_methods[]{reinterpret_cast<Dword>(state_base_delete)};
const Dword state_construct_methods[]{
    reinterpret_cast<Dword>(initial_delete), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call), reinterpret_cast<Dword>(block_bytes)};
const Dword state_initial_methods[]{
    reinterpret_cast<Dword>(initial_base_delete), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call), reinterpret_cast<Dword>(block_bytes)};
const Dword encrypt_methods[]{
    reinterpret_cast<Dword>(encrypt_delete), reinterpret_cast<Dword>(encrypt_entry),
    reinterpret_cast<Dword>(encrypt_in_place), reinterpret_cast<Dword>(block_bytes)};
const Dword decrypt_methods[]{
    reinterpret_cast<Dword>(decrypt_delete), reinterpret_cast<Dword>(decrypt_entry),
    reinterpret_cast<Dword>(decrypt_in_place), reinterpret_cast<Dword>(block_bytes)};
const Dword codec_methods[]{
    reinterpret_cast<Dword>(codec_delete),   reinterpret_cast<Dword>(ignore_error),
    reinterpret_cast<Dword>(allocate_entry), reinterpret_cast<Dword>(release_entry),
    reinterpret_cast<Dword>(encode_entry),   reinterpret_cast<Dword>(decode_entry),
    reinterpret_cast<Dword>(free_entry)};
const Dword codec_base_methods[]{reinterpret_cast<Dword>(partial_codec_delete)};
const Dword codec_abstract_methods[]{
    reinterpret_cast<Dword>(base_codec_delete), reinterpret_cast<Dword>(throw_error),
    reinterpret_cast<Dword>(base_allocate),     reinterpret_cast<Dword>(base_release),
    reinterpret_cast<Dword>(encode_entry),      reinterpret_cast<Dword>(decode_entry),
    reinterpret_cast<Dword>(free_entry)};
} // namespace nfs::network::cipher
