#include "nfs/audio/music/music_clients.hpp"
namespace nfs::game::audio {
using namespace state;

void reset_engine_state() {

    sound::field<Dword>(engine_storage, 244) = 0;
    sound::field<Byte>(engine_storage, 252) = 0;
    for (unsigned event = 0; event < 5; ++event)
        for (unsigned listener = 0; listener < 2; ++listener) {
            Byte *record = engine_storage + listener * 120 + event * 4;
            sound::field<Dword>(record, 0) = 0;
            sound::field<Dword>(record, 20) = 0;
            sound::field<Dword>(record, 40) = 240;
            sound::field<Dword>(record, 60) = 120;
            sound::field<Dword>(record, 80) = Dword(-1);
            sound::field<Dword>(record, 100) = 0;
        }
}
} // namespace nfs::game::audio
