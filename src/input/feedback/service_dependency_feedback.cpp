#include "nfs/core/services/service_dependencies.hpp"
#include <cstring>
namespace nfs::services::dependency {
namespace {
Byte *record(int slot) {
    return input::effect_record_data() + Dword(slot) * 148;
}
bool valid_effect(int slot) {
    return slot >= 0 && slot < 32 && input::effect_slots()[slot].device_index != -1;
}
bool retry_result(HRESULT result) {
    return result == HRESULT(2147942412u) || result == HRESULT(2147942430u) ||
           result == HRESULT(2147746309u);
}
} // namespace
// Only AL is consumed by callers.
Byte enable_feedback(Dword device) {
    auto *about = static_cast<input::DeviceAbout *>(input::device_get(static_cast<int>(device), 2));
    if (!(about->flags & 2) || (input::devices[device].flags & 1))
        return 0;
    Byte result = input::reset_effects(static_cast<int>(device));
    if (result == 1)
        input::devices[device].flags |= 1;
    return result;
}

int find_unused_effect() {
    for (int slot = 0; slot < 32; ++slot)
        if (input::effect_slots()[slot].device_index == -1)
            return slot;
    return -1;
}

void download_effect_with_retry(IDirectInputDevice2A *device, IDirectInputEffect *effect) {
    const HRESULT result = effect->Download();
    if (retry_result(result) && device->Acquire() == 0)
        effect->Download();
}

void download_effect(int slot) {
    if (valid_effect(slot)) {
        auto &effect = input::effect_slots()[slot];
        download_effect_with_retry(input::devices[effect.device_index].device, effect.effect);
    }
}

void unload_effect(int slot) {
    if (valid_effect(slot))
        input::effect_slots()[slot].effect->Unload();
}

void set_feedback_gain(Dword device, Dword gain) {
    if (static_cast<int>(device) < 0 || static_cast<int>(device) >= input::device_count ||
        !input::devices[device].device || !(input::devices[device].flags & 1))
        return;
    const int requested = static_cast<int>(gain);
    DIPROPDWORD property{{20, 16, 0, 0},
                         requested > 100 ? 10000u
                         : requested < 0 ? 0u
                                         : gain * 100u};
    for (int slot = 0; slot < 32; ++slot)
        if (input::effect_slots()[slot].device_index != -1)
            unload_effect(slot);
    using SetProperty = HRESULT(__stdcall *)(IDirectInputDevice2A *, Dword, const DIPROPHEADER *);
    (*reinterpret_cast<const SetProperty **>(input::devices[device].device))[6](
        input::devices[device].device, 7, &property.diph);
    for (int slot = 0; slot < 32; ++slot)
        if (input::effect_slots()[slot].device_index != -1)
            download_effect(slot);
}

Dword create_feedback_effect(Dword device, Dword *definition) {
    const int slot = find_unused_effect();
    if (slot == -1 || static_cast<int>(device) < 0 ||
        static_cast<int>(device) >= input::device_count || !input::devices[device].device ||
        !(input::devices[device].flags & 1))
        return Dword(-1);
    Byte *effect = record(slot);
    field<Dword>(effect, 8) = definition[13];
    field<Dword>(effect, 16) = definition[14] * 100;
    field<Dword>(effect, 20) = definition[16] == Dword(-1) ? Dword(-1) : definition[16] + 48;
    field<Dword>(effect, 24) = definition[17];
    field<Dword>(effect, 124) = definition[18] * 100;
    Dword axes[2];
    if (definition[15] == 1) {
        field<Dword>(effect, 28) = 1;
        axes[0] = 0;
    } else if (definition[15] == 2) {
        field<Dword>(effect, 28) = 1;
        axes[0] = 4;
    } else {
        field<Dword>(effect, 28) = 2;
        axes[0] = 0;
        axes[1] = 4;
    }
    field<Dword *>(effect, 32) = axes;
    std::memcpy(effect + 56, definition + 1, 48);
    field<Dword>(effect, 4) = 34;
    field<Dword>(effect, 12) = 2000;
    field<Dword>(effect, 0) = 56;
    field<void *>(effect, 36) = effect + 124;
    for (unsigned offset : {128u, 40u, 132u})
        field<Dword>(effect, offset) = 0;
    field<void *>(effect, 48) = effect + 56;
    field<Dword>(effect, 136) = definition[0];
    field<Dword>(effect, 144) = device;
    switch (definition[0]) {
    case 0:
        field<Dword>(effect, 44) = 4;
        break;
    case 1:
        field<Dword>(effect, 44) = 8;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        field<Dword>(effect, 44) = 16;
        break;
    case 8:
    case 10:
    case 11:
    case 12:
        field<Dword>(effect, 44) = 24;
        break;
    case 9:
        field<Dword>(effect, 44) = 48;
        field<Dword>(effect, 124) = 9000;
        break;
    default:
        return Dword(-1);
    }
    using Create = HRESULT(__stdcall *)(IDirectInputDevice2A *, const GUID *, const void *,
                                        IDirectInputEffect **, void *);
    IDirectInputDevice2A *owner = input::devices[device].device;
    const HRESULT result = (*reinterpret_cast<const Create **>(owner))[72 / 4](
        owner, &pending::effect_guids[definition[0]], effect, &input::effect_slots()[slot].effect,
        nullptr);
    if (result != 0)
        return Dword(-1);
    unload_effect(slot);
    return static_cast<Dword>(slot);
}
} // namespace nfs::services::dependency
