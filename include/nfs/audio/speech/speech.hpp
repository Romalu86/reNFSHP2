#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::speech {
using kernel::Dword;
using Byte = std::uint8_t;
using EventSink = void(__cdecl *)(Dword, int, ...);
extern Byte random_storage[92], control_storage[1168], queue_storage[2024];
inline Byte *state_address(unsigned va) {
    if (va >= 8767768)
        return queue_storage + (va - 8767768);
    if (va >= 8739144)
        return control_storage + (va - 8739144);
    return random_storage + (va - 7146808);
}
template <class T> T &state(unsigned va) {
    return *reinterpret_cast<T *>(state_address(va));
}
extern EventSink &event_sink;
Dword create_event_code(Dword, Dword, Dword);
void flush_events(Dword);
} // namespace nfs::speech
#include "nfs/audio/speech/speech_queue.hpp"
#include "nfs/audio/speech/speech_rules.hpp"
#include "nfs/audio/speech/speech_interpreter.hpp"

#include "nfs/audio/speech/speech_game.hpp"
