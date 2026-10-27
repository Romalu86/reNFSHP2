#include "nfs/audio/speech/speech_game.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::speech::game {

Dword speed_phrase_mask(int speed) {
    if (network::packets::rule_value(nfs::game::runtime::system(72), 44)) {
        const long double metric_speed = static_cast<long double>(speed) * 1.6093100309371948f;
        speed = int(Dword(nfs::game::runtime::hardware::truncate_integer_native80(&metric_speed)));
    }
    if (speed >= 340)
        return 4096;
    if (speed >= 320)
        return 2048;
    if (speed >= 300)
        return 1024;
    if (speed >= 280)
        return 512;
    if (speed >= 260)
        return 256;
    if (speed >= 240)
        return 128;
    if (speed >= 220)
        return 64;
    if (speed >= 200)
        return 32;
    if (speed >= 180)
        return 16;
    if (speed >= 160)
        return 8;
    if (speed >= 140)
        return 4;
    if (speed >= 120)
        return 2;
    return speed >= 100;
}
} // namespace nfs::speech::game
