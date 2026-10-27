#include "nfs/audio/effects/sound_reverb.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend {
extern const Byte effect_capabilities_guid[16];
extern const float environment_size_scale, environment_decay_scale;

int apply_environment(int kind, int room, int decay) {
    platform_tail<int>(8717644) = kind;
    platform_tail<int>(8717648) = room;
    platform_tail<int>(8717652) = decay;
    if (!use_effect_interface)
        return -1;
    int environment = 9;
    float volume = 1;
    if (kind == 0 || kind == 1) {
        room = decay = -1;
        volume = 0;
        environment = 1;
    } else if (kind == 2)
        environment = 1;
    else {
        switch (kind) {
        case 3:
            environment = 11;
            break;
        case 5:
            environment = 4;
            break;
        case 10:
            environment = 2;
            break;
        case 13:
            environment = 19;
            break;
        case 15:
            environment = 15;
            break;
        case 17:
            environment = 17;
            break;
        case 20:
            environment = 16;
            break;
        case 23:
            environment = 20;
            break;
        case 25:
            environment = 12;
            break;
        case 27:
            environment = 14;
            break;
        case 30:
            environment = 0;
            break;
        case 32:
            environment = 5;
            break;
        case 33:
            environment = 3;
            break;
        case 34:
            environment = 18;
            break;
        case 35:
            environment = 21;
            break;
        case 36:
            environment = 8;
            break;
        case 37:
            environment = 13;
            break;
        case 38:
            environment = 6;
            break;
        case 40:
            environment = 7;
            break;
        case 50:
            environment = 9;
            break;
        case 55:
            environment = 10;
            break;
        case 60:
            environment = 22;
            break;
        case 70:
            environment = 24;
            break;
        case 73:
            environment = 23;
            break;
        case 76:
            environment = 25;
            break;
        default:
            if (kind > 99 && kind < 121)
                environment = 10;
            break;
        }
    }
    if (device_call<int>(platform_tail<void *>(8717636), 16, effect_capabilities_guid, 1, 0, 0,
                         &environment, 4))
        return -1;
    if (device_call<int>(platform_tail<void *>(8717636), 16, effect_capabilities_guid, 2, 0, 0,
                         &volume, 4))
        return -1;
    if (room != -1) {
        float value = float(static_cast<long double>(room) * environment_size_scale);
        if (device_call<int>(platform_tail<void *>(8717636), 16, effect_capabilities_guid, 3,
                             0, 0, &value, 4))
            return -1;
    }
    if (decay != -1) {
        float value = float(static_cast<long double>(decay) * environment_decay_scale);
        if (device_call<int>(platform_tail<void *>(8717636), 16, effect_capabilities_guid, 4,
                             0, 0, &value, 4))
            return -1;
    }
    return 0;
}

void update_effect_group(Byte mask) {
    lock();
    if (mask & 16)
        apply_environment(field<int>(effect_groups, 20), field<int>(effect_groups, 24),
                          field<int>(effect_groups, 28));
    else if (mask & 4) {
        if (!field<Dword>(effect_groups, 0))
            effects::reset();
        else
            effects::configure(setting<std::uint16_t>(8708898),
                               field<Dword>(effect_groups, 0) == 1
                                   ? field<const void *>(effect_groups, 4)
                                   : effects::tables);
    }
    unlock();
}

int refresh_effect_group(Dword, Dword mask) {
    lifecycle<void(__cdecl *)(Byte)>(8708836) = update_effect_group;
    update_effect_group(Byte(mask));
    return 0;
}

int set_effect_group_flags(Dword flags, Dword value) {
    if (!control.initialized)
        return -10;
    Dword group = flags & 4294966791u;
    if (!(flags & 504))
        flags |= 504;
    if (flags & 16)
        field<Byte>(effect_groups, group * 20 + 2) = Byte(value);
    if (flags & 384)
        field<Byte>(effect_groups, group * 20 + 22) = Byte(value);
    lock();
    for (int i = 0; i < control.voice_count; ++i)
        set_voice_effect(field<Dword>(control.voices + i, 0), int(group),
                         field<Byte>(control.voices + i, 85 + group));
    unlock();
    return 0;
}

int configure_effect_groups(Dword flags, Dword enabled, int kind, Dword first, Dword second) {
    if (!control.initialized)
        return -10;
    if (!(flags & 504))
        flags |= 504;
    Dword mask = effect_mask(Byte(flags)), group = flags & 4294966791u;
    for (Dword bit = 4; mask; bit <<= 1)
        if (mask & bit) {
            void *entry = effect_group(int(group), bit);
            field<std::uint16_t>(entry, 0) = std::uint16_t(kind);
            field<Dword>(entry, 4) = first;
            field<Dword>(entry, 8) = second;
            refresh_effect_group(group, bit);
            mask &= ~bit;
        }
    set_effect_group_flags(flags, kind ? enabled : 0);
    return 0;
}
} // namespace nfs::sound::backend
