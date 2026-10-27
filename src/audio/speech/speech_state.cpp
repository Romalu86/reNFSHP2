#include "nfs/audio/speech/speech.hpp"
namespace nfs::speech {
#include "speech_data.inc"
EventSink &event_sink = *reinterpret_cast<EventSink *>(control_storage + 20);

Dword create_event_code(Dword source, Dword target, Dword event) {
    return ((source & 255) << 24) | ((target & 255) << 16) | (event & 65535);
}
} // namespace nfs::speech
namespace nfs::game::audio::vehicle::pending {
void(__cdecl *&speech_sink)(nfs::kernel::Dword, int, ...) =
    *reinterpret_cast<nfs::speech::EventSink *>(nfs::speech::control_storage + 20);
void flush_speech_events(nfs::kernel::Dword voice) {
    nfs::speech::flush_events(voice);
}
} // namespace nfs::game::audio::vehicle::pending
