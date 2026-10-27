#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/audio/backend/sound_backend.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend {

void chain_set_value(void *context, Dword value) {
    if (void *resampler = field<void *>(backends + field<std::int16_t>(context, 2), 16))
        pending::seek_resampler(resampler, value);
}

void chain_set_loop(void *context, Dword value) {
    int voice = field<std::int16_t>(context, 2);
    auto *owner = backends + voice;
    Dword rate = field<std::uint16_t>(control.voices + voice, 28);
    float ratio =
        float(static_cast<long double>(int(value)) / (static_cast<long double>(rate) * 0.5f));
    if (ratio < 1) {
        if (!field<void *>(owner, 20)) {
            void *filter = allocate(40);
            field<void *>(owner, 20) = filter;
            pending::create_filter(filter);
            field<std::uint16_t>(filter, 24) = 2;
            insert_processor(&field<void *>(owner, 212), filter);
        }
        int parameters[3] = {int(value << 8), int(rate << 8), 256};
        pending::update_filter(field<void *>(owner, 20), parameters);
    } else if (void *filter = field<void *>(owner, 20)) {
        remove_processor(&field<void *>(owner, 212), filter);
        release(filter);
        field<void *>(owner, 20) = nullptr;
    }
}

void chain_set_effect(void *context, Dword value) {
    int voice = field<std::int16_t>(context, 2);
    auto *owner = backends + voice;
    if (int(value) > 0) {
        if (!field<void *>(owner, 24)) {
            void *effect = allocate(88);
            field<void *>(owner, 24) = effect;
            pending::create_effect(effect);
            field<std::uint16_t>(effect, 24) = 3;
            insert_processor(&field<void *>(owner, 212), effect);
        }
        int parameters[2] = {int(value << 8),
                             int(Dword(field<std::uint16_t>(control.voices + voice, 28)) << 8)};
        pending::change_effect(field<void *>(owner, 24), parameters);
    } else if (void *effect = field<void *>(owner, 24)) {
        remove_processor(&field<void *>(owner, 212), effect);
        release(effect);
        field<void *>(owner, 24) = nullptr;
    }
}

int chain_remaining(void *context) {
    if (auto method = field<Dword>(context, 4))
        return reinterpret_cast<int(__cdecl *)(void *)>(method)(
            field<void *>(backends + field<std::int16_t>(context, 2), 8));
    return -1;
}
namespace pending {
extern int process_filter(void *, int, void *, void *, Byte);
extern int process_effect(void *, int, void *, void *, Byte);
extern void configure_effect(void *, int);
extern const float filter_gain_scale;

int create_filter(void *processor) {
    for (unsigned offset = 8; offset <= 20; offset += 4)
        field<Dword>(processor, offset) = 0;
    field<Byte>(processor, 27) = 0;
    field<Dword>(processor, 4) = 0;
    field<Dword>(processor, 0) = reinterpret_cast<Dword>(process_filter);
    field<Dword>(processor, 28) = 0;
    return 0;
}

void update_filter(void *processor, const int *parameters) {

    auto I = [&](int value) { return static_cast<long double>(value); };
    const auto numerator = I(parameters[0]);
    const auto ratio = (numerator + numerator) / I(parameters[1]);
    field<float>(processor, 36) = float(ratio);
    field<float>(processor, 32) = float(static_cast<long double>(1.f) - ratio);
    field<float>(processor, 36) = float(I(parameters[2]) *
                                          field<float>(processor, 36) * filter_gain_scale);
    field<float>(processor, 32) = float(I(parameters[2]) *
                                          field<float>(processor, 32) * filter_gain_scale);
}

void reset_effect(void *state) {
    for (unsigned i = 0; i < 8; ++i)
        field<Dword>(state, i * 4) = 0;
}

int create_effect(void *processor) {
    for (unsigned offset = 8; offset <= 20; offset += 4)
        field<Dword>(processor, offset) = 0;
    field<Byte>(processor, 27) = 0;
    field<Dword>(processor, 4) = 0;
    field<Dword>(processor, 0) = reinterpret_cast<Dword>(process_effect);
    reset_effect(static_cast<Byte *>(processor) + 28);
    return 0;
}

void change_effect(void *processor, const int *parameters) {
    field<float>(processor, 80) =
        float(static_cast<long double>(parameters[0] >> 7) / (parameters[1] >> 8));
    configure_effect(static_cast<Byte *>(processor) + 28, 3);
}
} // namespace pending
} // namespace nfs::sound::backend
