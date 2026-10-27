#include "../screens/frontend_menu_roots.hpp"
#include "nfs/audio/speech/speech.hpp"
namespace audio_layout = nfs::game::audio::state::layout;
namespace nfs::application::frontend {
using game::runtime::at;

Dword car_description_mask(Dword car) {
    switch (car) {
    case 0:
    case 38:
    case 62:
    case 78:
        return 16;
    case 1:
    case 39:
    case 63:
        return 67108864;
    case 2:
    case 40:
    case 64:
        return 256;
    case 3:
    case 41:
    case 65:
        return 2048;
    case 4:
    case 42:
    case 66:
        return 8;
    case 5:
    case 43:
        return 262144;
    case 6:
    case 44:
    case 69:
        return 1;
    case 8:
    case 46:
    case 70:
        return 64;
    case 9:
    case 47:
    case 71:
        return 32768;
    case 10:
    case 48:
    case 72:
    case 76:
        return 2;
    case 11:
    case 49:
    case 73:
        return 33554432;
    case 12:
    case 50:
    case 74:
        return 128;
    case 13:
    case 51:
    case 75:
        return 1048576;
    case 14:
    case 52:
        return 8192;
    case 15:
    case 53:
    case 77:
        return 512;
    case 16:
    case 54:
    case 68:
        return 4;
    case 17:
    case 55:
    case 79:
        return 65536;
    case 18:
    case 56:
    case 80:
        return 524288;
    case 19:
    case 57:
    case 81:
        return 4194304;
    case 20:
    case 58:
    case 67:
    case 82:
        return 32;
    case 21:
    case 59:
    case 83:
        return 131072;
    case 22:
    case 60:
    case 84:
        return 4096;
    case 23:
    case 61:
    case 85:
        return 1024;
    default:
        return 0;
    }
}

void play_car_description(void *owner, Dword car) {
    if (!at<Dword>(owner, 20))
        return;
    if (game::audio::state::option<Dword>(audio_layout::speech_stream_active))
        sound::streaming::stop_stream(at<Dword>(owner, 16));
    const Dword mask = car_description_mask(car);
    if (car == 67 || car == 68 || car == 69 || car == 76 || car == 78)
        speech::event_sink(speech::create_event_code(0, 0, 211), 1, mask);
    else
        speech::event_sink(speech::create_event_code(0, 0, 146), 2,
                           car >= 38 && car < 62 && car != 53 ? 1 : 2, mask);
    speech::flush_events(0);
}
} // namespace nfs::application::frontend
