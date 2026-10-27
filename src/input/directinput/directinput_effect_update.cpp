#include "nfs/input/directinput/directinput.hpp"
#include "nfs/core/services/service_dependencies.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
namespace nfs::input {
namespace {
template <class T> T &effect_field(int index, unsigned offset) {
    return *reinterpret_cast<T *>(effect_record_data() + Dword(index) * 148 + offset);
}
} // namespace
// The device Acquire result replaces the first error.
HRESULT start_effect_with_retry(IDirectInputDevice2A *device, IDirectInputEffect *effect,
                                Dword count, Dword flags) {
    HRESULT result = effect->Start(count, flags);
    if (result == HRESULT(2147942412u) || result == HRESULT(2147942430u) ||
        result == HRESULT(2147746309u)) {
        result = device->Acquire();
        if (result == 0)
            result = effect->Start(count, flags);
    }
    return result;
}

void start_effect(int index) {
    if (index >= 0 && index < 32 && effect_slots()[index].device_index != -1) {
        auto &effect = effect_slots()[index];
        const HRESULT result =
            start_effect_with_retry(devices[effect.device_index].device, effect.effect, 1, 0);
        effect_stop_state(index) = result == 0 || result == 1;
    }
}
// The entry has no upper-bound test.
Dword infinite_effect_active(int index) {
    if (index < 0 || effect_field<Dword>(index, 8) != Dword(-1))
        return 0;
    return effect_stop_state(index);
}
// Only AL is part of the defined return contract.
std::uint8_t update_effect(int index, int action, const void *parameters, Dword *duration,
                           const int *gain, const int *trigger, const int *direction,
                           const int *envelope) {
    if (index < 0 || index >= 32 || effect_slots()[index].device_index == -1)
        return 0;
    Dword changes = 0;
    if (direction && effect_field<Dword>(index, 124) != Dword(*direction) * 100) {
        effect_field<Dword>(index, 124) = Dword(*direction) * 100;
        changes = 64;
    }
    if (duration) {
        auto &requested_duration = *static_cast<volatile Dword *>(duration);
        const Dword requested = requested_duration;
        if (requested == Dword(-1))
            requested_duration = requested;
        else if (requested > 1500000)
            requested_duration = 1500000;
        if (effect_field<Dword>(index, 8) != *duration) {
            effect_field<Dword>(index, 8) = *duration;
            changes |= 1;
        }
    }
    if (gain && effect_field<Dword>(index, 16) != Dword(*gain) * 100) {
        effect_field<Dword>(index, 16) = Dword(*gain) * 100;
        changes |= 4;
    }
    auto *record = effect_record_data() + Dword(index) * 148;
    if (parameters) {
        const Dword bytes = effect_field<Dword>(index, 44);
        if (std::memcmp(record + 56, parameters, bytes)) {
            kernel::memory_copy(record + 56, parameters, bytes);
            changes |= 256;
        }
    }
    if (trigger) {
        effect_field<Dword>(index, 20) = *trigger == -1 ? Dword(-1) : Dword(*trigger) + 48;
        changes |= 8;
    }
    if (envelope && (effect_field<int>(index, 108) != envelope[0] ||
                     effect_field<int>(index, 112) != envelope[1] ||
                     effect_field<int>(index, 116) != envelope[2] ||
                     effect_field<int>(index, 120) != envelope[3])) {
        for (unsigned field = 0; field < 4; ++field)
            effect_field<int>(index, 108 + field * 4) = envelope[field];
        changes |= 128;
    }
    if (changes) {
        using SetParameters = HRESULT(__stdcall *)(IDirectInputEffect *, const void *, Dword);
        IDirectInputEffect *effect = effect_slots()[index].effect;
        (*reinterpret_cast<const SetParameters **>(effect))[6](effect, record, changes);
        if (action == 3)
            services::dependency::download_effect(index);
        else if ((action == 1 || action == 2) && !infinite_effect_active(index))
            start_effect(index);
        return 1;
    }
    if (action == 1) {
        if (!infinite_effect_active(index))
            start_effect(index);
    } else if (action == 3)
        services::dependency::download_effect(index);
    return 0;
}
} // namespace nfs::input
