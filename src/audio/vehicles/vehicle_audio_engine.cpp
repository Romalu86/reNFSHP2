#include "nfs/audio/vehicles/vehicle_audio.hpp"
namespace nfs::game::audio::vehicle {

void mix_engine_owner(int listener) {
    if (!owner(listener))
        return;
    sound::lock();
    auto *engine = owner(listener);
    at<std::int16_t>(engine, 20) = at<std::int16_t>(engine, 24) =
        at<std::int16_t>(engine, 22);
    for (int layer = 0; layer < at<std::int8_t>(owner(listener), 19); ++layer) {
        engine = owner(listener);
        int side = layer > 7, table_slot = side ? layer - 8 : layer;
        Byte *curve = table(7218904, listener * 2 + side);
        int patch = at<std::int8_t>(curve, 32 + table_slot);
        if (patch < 0 || bank(listener, patch).handle == -1)
            continue;
        Byte *volume_curve = at<Byte *>(curve, 296 + table_slot * 4);
        int layer_gain = at<std::int8_t>(volume_curve, at<std::int16_t>(engine, 24));
        Byte normalized = Byte(static_cast<int>(
            static_cast<long double>(at<std::int16_t>(engine, 20)) * 0.00227272720076143741607666015625f * 127.0f));
        int cents = (((int(normalized) - 64) * at<int>(engine, 28)) >> 6) +
                    bank(listener, patch).pitch + at<int>(engine, 32);
        int multiplier = 4096;
        if (cents > 1199) {
            Dword octaves = Dword(cents) / 1200;
            cents = Dword(cents) % 1200;
            do {
                multiplier = int(Dword(multiplier) << 1);
            } while (--octaves);
        }
        if (cents < -1199) {
            int octaves = (-cents - 1200) / 1200 + 1;
            cents += octaves * 1200;
            do {
                multiplier >>= 1;
            } while (--octaves);
        }
        int curve_index = wrapped_product(cents, 13981) >> 16;
        if (curve_index < -255)
            curve_index = -255;
        if (curve_index < 0)
            multiplier =
                wrapped_product(int(engine_curves[392 + curve_index]) + 256, multiplier) >> 9;
        else
            multiplier =
                wrapped_product(int(engine_curves[136 + curve_index]) + 256, multiplier) >> 8;
        int rate = wrapped_product(at<std::uint16_t>(engine, 12), multiplier) >> 12;
        rate = clamp_value(rate, 0, 65535);
        int throttle = at<int>(engine, 8);
        int weight = engine_curves[side ? throttle : 127 - throttle];
        int volume =
            wrapped_product(weight, wrapped_product(at<int>(engine, 4), layer_gain) >> 7) >> 7;
        unsigned primary = 44 + layer * 12, secondary = 236 + layer * 12;
        bool stereo = at<int>(runtime::system(72), 176) == 1;
        if (!volume) {
            if (at<int>(engine, primary + 8) != -1) {
                sound::stop(at<Dword>(engine, primary + 8));
                at<int>(owner(listener), primary + 8) = -1;
                if (stereo) {
                    sound::stop(at<Dword>(owner(listener), secondary + 8));
                    at<int>(owner(listener), secondary + 8) = -1;
                }
            }
            continue;
        }
        volume = wrapped_product(volume, 5) >> 2;
        if (volume > 127)
            volume = 127;
        Byte blend = at<Byte>(engine, 18);
        weight = engine_curves[bank(listener, patch).exhaust ? blend : 127 - blend];
        int mixed = wrapped_product(weight, volume) >> 7;
        Dword center = at<std::uint16_t>(engine, 14), spread = at<std::uint16_t>(engine, 16);
        std::uint16_t left = std::uint16_t(stereo ? center - spread : center),
                      right = std::uint16_t(center + spread);
        sound::VoiceParameters parameters;
        sound::initialize_parameters(&parameters);
        at<Byte>(&parameters, 0) = 0;
        at<Byte>(&parameters, 1) = 64;
        at<std::uint16_t>(&parameters, 10) = 0;
        at<std::uint16_t>(&parameters, 12) = std::uint16_t(rate);
        auto start = [&](unsigned record, std::uint16_t pan) {
            at<std::uint16_t>(&parameters, 8) = pan;
            auto *current = owner(listener);
            Dword handle = sound::play_bank(bank(listener, patch).handle, 0, &parameters);
            at<Dword>(current, record + 8) = handle;
            at<Byte>(current, record) = at<Byte>(&parameters, 0);
            at<Byte>(current, record + 1) = at<Byte>(&parameters, 1);
            at<std::uint16_t>(current, record + 4) = pan;
            at<std::uint16_t>(current, record + 2) = at<std::uint16_t>(&parameters, 12);
            sound::set_voice_effect(at<Dword>(current, record + 8), 0, 127);
        };
        engine = owner(listener);
        if (stereo) {
            int mode = at<int>(runtime::system(72), 2360);
            if ((at<int>(engine, secondary + 8) == -1 && mode != 2) ||
                (at<int>(engine, primary + 8) == -1 && mode == 2)) {
                if (at<int>(engine, primary + 8) == -1) {
                    at<Byte>(engine, 428 + layer) = 2;
                    start(primary, left);
                } else
                    --at<Byte>(engine, 428 + layer);
                engine = owner(listener);
                if (at<int>(engine, secondary + 8) == -1 && !at<Byte>(engine, 428 + layer))
                    start(secondary, right);
                continue;
            }
        } else if (at<int>(engine, primary + 8) == -1) {
            start(primary, left);
            continue;
        }
        engine = owner(listener);
        if (at<std::int8_t>(engine, primary) != mixed) {
            std::int8_t value = std::int8_t(mixed);
            at<Byte>(engine, primary) = value < 0 ? 0 : Byte(value);
            if (stereo)
                at<Byte>(engine, secondary) = value < 0 ? 0 : Byte(value);
            sound::set_voice_volume(at<Dword>(engine, primary + 8),
                                    at<std::int8_t>(engine, primary));
            if (stereo)
                sound::set_voice_volume(at<Dword>(owner(listener), secondary + 8),
                                        at<std::int8_t>(owner(listener), secondary));
        }
        engine = owner(listener);
        if (at<std::uint16_t>(engine, primary + 4) != left) {
            sound::set_voice_pan(at<Dword>(engine, primary + 8), left, 0);
            at<std::uint16_t>(owner(listener), primary + 4) = left;
            if (stereo) {
                sound::set_voice_pan(at<Dword>(owner(listener), secondary + 8), right, 0);
                at<std::uint16_t>(owner(listener), secondary + 4) = right;
            }
        }
        engine = owner(listener);
        if (at<std::uint16_t>(engine, primary + 2) != rate) {
            sound::set_voice_rate(at<Dword>(engine, primary + 8), rate);
            at<std::uint16_t>(owner(listener), primary + 2) = std::uint16_t(rate);
            if (stereo) {
                sound::set_voice_rate(at<Dword>(owner(listener), secondary + 8), rate);
                at<std::uint16_t>(owner(listener), secondary + 2) = std::uint16_t(rate);
            }
        }
    }
    sound::unlock();
}

void mix_engine_owners() {
    mix_engine_owner(0);
    mix_engine_owner(1);
}
} // namespace nfs::game::audio::vehicle

namespace nfs::game::runtime::race_audio::pending {
void reset_environment() {
    audio::vehicle::mix_engine_owners();
}
} // namespace nfs::game::runtime::race_audio::pending
