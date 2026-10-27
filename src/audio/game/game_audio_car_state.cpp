#include "nfs/audio/music/music_clients.hpp"
namespace nfs::game::audio {

void reset_car_audio(void *car) {
    sound::field<Dword>(car, 1792) &= 4294963200u;
}
} // namespace nfs::game::audio
