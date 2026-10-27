#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::pending {
extern void construct_rate(void *, Byte, int);

void set_processor_rate(int index, Dword rate) {
    if (int(rate) > 262144)
        rate = 262144;
    auto *voice = mix_voices + index;
    if (!field<void *>(voice, 64)) {
        void *processor = allocate(60);
        field<void *>(voice, 64) = processor;
        field<Dword>(processor, 4) = 0;
        field<std::uint16_t>(processor, 24) = 160;
        construct_rate(processor, control.unknown_7, mix_format[2] > 49);
        insert_processor(&field<void *>(voice, 48), field<void *volatile>(voice, 64));
    }
    field<Dword>(field<void *>(voice, 64), 28) = rate;
}

void set_rate(void *processor, Dword rate) {
    field<Dword>(processor, 28) = rate;
}

void set_processor_seek(int index, Dword value) {
    if (void *resampler = field<void *>(mix_voices + index, 60))
        seek_resampler(resampler, value);
}

void seek_resampler(void *processor, Dword value) {

    const auto scaled = static_cast<long double>(int(value)) *
                        backend::device_rate_scale;
    field<float>(processor, 36) = float(scaled);
    if (scaled < static_cast<long double>(0.5f))
        field<Dword>(processor, 36) = 1056964608;
    else if (static_cast<long double>(2.f) < scaled)
        field<Dword>(processor, 36) = 1073741824;
}

void set_processor_loop(int index, float value) {
    auto *voice = mix_voices + index;
    if (value < 1) {
        if (!field<void *>(voice, 72)) {
            void *filter = allocate(40);
            field<void *>(voice, 72) = filter;
            field<Dword>(filter, 4) = 0;
            field<std::uint16_t>(filter, 24) = 40;
            create_filter(filter);
            insert_processor(&field<void *>(voice, 48), field<void *volatile>(voice, 72));
        }
        Dword rate = field<std::uint16_t>(&nfs::sound::pending::bank_limits, 2);
        int parameters[3] = {int(Dword(std::int64_t(static_cast<long double>(rate) * value)) << 7),
                             int(rate << 8), 256};
        update_filter(field<void *>(voice, 72), parameters);
    } else if (void *filter = field<void *>(voice, 72)) {
        remove_processor(&field<void *>(voice, 48), filter);
        release(field<void *volatile>(voice, 72));
        field<void *>(voice, 72) = nullptr;
    }
}

void set_processor_effect(int index, Dword value) {
    auto *voice = mix_voices + index;
    if (int(value) > 0) {
        if (!field<void *>(voice, 76)) {
            void *effect = allocate(88);
            field<void *>(voice, 76) = effect;
            field<Dword>(effect, 4) = 0;
            field<std::uint16_t>(effect, 24) = 80;
            create_effect(effect);
            insert_processor(&field<void *>(voice, 48), field<void *volatile>(voice, 76));
        }
        int parameters[2] = {
            int(value << 8),
            int(Dword(field<std::uint16_t>(&nfs::sound::pending::bank_limits, 2)) << 8)};
        change_effect(field<void *>(voice, 76), parameters);
    } else if (void *effect = field<void *>(voice, 76)) {
        remove_processor(&field<void *>(voice, 48), effect);
        release(field<void *volatile>(voice, 76));
        field<void *>(voice, 76) = nullptr;
    }
}

int processor_position(int index) {
    auto *voice = mix_voices + index;
    if (auto query = field<Dword>(voice, 52))
        return reinterpret_cast<int(__cdecl *)(void *)>(query)(field<void *>(voice, 56));
    return -1;
}
} // namespace nfs::sound::backend::pending
