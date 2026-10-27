#include "nfs/audio/backend/sound_ownership.hpp"
namespace nfs::sound::backend {
void render_mixer(void *, int);
void update_audio_voices();

void service_hardware_streams() {
    for (int i = setting<Byte>(8708902); i < std::int16_t(control.voice_count); ++i) {
        BackendVoice *voice = backends + i;
        if (!field<void *>(voice, 212))
            continue;
        for (;;) {
            using Remaining = int(__cdecl *)(void *);
            void *context = field<void *>(voice, 204);
            int available = field<Remaining>(voice, 248)(context);
            int count = available > 256 ? 256 : available;
            void *node = field<void *>(voice, 212);
            int result = field<pending::Processor>(node, 0)(
                node, count, lifecycle<void *>(8708828), lifecycle<void *>(8708832), 0);
            if (result < 0) {
                stop_voice(i);
                break;
            }
            dispatch_stream_events();
            if (available - 256 <= 0)
                break;
        }
    }
}
// Return-register scratch is not consumed by its callers.
void adapt_output_latency() {
    auto &produced = platform<Dword>(8711396), &consumed = platform<Dword>(8711400),
         &latency = platform<Dword>(8711416), &period = platform<Dword>(8711436);
    auto &minimum = platform<int>(8711432);
    Dword ring = platform<Dword>(8711452);
    if (produced > 285212672) {
        produced += 4026531840u;
        consumed -= 268435456;
    }
    int buffered = int(produced - consumed);
    if (buffered >= int(ring)) {
        produced -= ring;
        buffered -= int(ring);
    }
    if (buffered < 0) {
        if (produced) {
            latency = (latency - Dword(buffered) + 15) & 1048560;
            if (int(latency) > platform<int>(8711420))
                latency = platform<Dword>(8711420);
            period = 0;
        }
    } else {
        ++period;
        if (buffered < minimum)
            minimum = buffered;
        int observed = minimum;
        if (int(period) > 2999 && int(period) % 10 == 0) {
            minimum = int(ring);
            if (observed > 16)
                latency = (latency - 1) & 1048560;
        }
    }
}

void prime_output_ring() {
    if (!direct_sound)
        return;
    Dword status;
    device_call<int>(platform<void *>(8711376), 36, &status);
    if ((status & 2) || platform_tail<int>(8717668) > 100) {
        device_call<int>(platform<void *>(8711376), 80);
        platform_tail<Dword>(8717668) = 0;
        return;
    }
    if (!(status & 1)) {
        device_call<int>(platform<void *>(8711376), 48, 0, 0, 1);
        platform_tail<Dword>(8717668) = 0;
        return;
    }
    if (device_call<int>(platform<void *>(8711376), 16, 0, &platform<Dword>(8711404)))
        return;
    Dword cursor = platform<Dword>(8711404) >> 1;
    if (!setting<Byte>(8708920))
        cursor /= Byte(output_channels);
    cursor &= 16777200;
    Dword previous = platform<Dword>(8711408), ring = platform<Dword>(8711452);
    platform<Dword>(8711404) = cursor;
    platform<Dword>(8711400) += cursor < previous ? ring - previous + cursor : cursor - previous;
    if (cursor == previous)
        ++platform_tail<Dword>(8717668);
    else
        platform_tail<Dword>(8717668) = 0;
    Dword target, amounts[2];
    for (;;) {
        target = (platform<Dword>(8711416) + cursor) & 1048560;
        if (int(target) >= int(ring))
            target -= ring;
        Dword write = platform<Dword>(8711412);
        if (int(target) < int(write)) {
            amounts[0] = ring - write;
            amounts[1] = target;
        } else {
            amounts[0] = target - write;
            amounts[1] = 0;
        }
        if (int(amounts[0] + amounts[1]) < int(ring - 16))
            break;
        platform<Dword>(8711416) += 16;
    }
    if (!platform<Byte>(8711392))
        adapt_output_latency();
    platform<Dword>(8711408) = platform<Dword>(8711404);
    for (unsigned part = 0; part < 2; ++part) {
        Dword amount = amounts[part];
        if (!amount)
            continue;
        platform<Dword>(8711396) += amount;
        void *buffers[4];
        Dword locked;
        if (setting<Byte>(8708920))
            for (unsigned c = 0; c < Byte(output_channels); ++c)
                device_call<int>(platform<void *>(8711376 + c * 4), 44,
                                 platform<Dword>(8711412) * 2, amount * 2, &buffers[c], &locked, 0,
                                 0, 0);
        else
            device_call<int>(platform<void *>(8711376), 44,
                             platform<Dword>(8711412) << (Byte(output_channels) & 31),
                             amount << (Byte(output_channels) & 31), buffers, &locked, 0, 0, 0);
        render_output(buffers, int(amount));
        if (setting<Byte>(8708920))
            for (unsigned c = 0; c < Byte(output_channels); ++c)
                device_call<int>(platform<void *>(8711376 + c * 4), 76, buffers[c], amount * 2,
                                 0, 0);
        else
            device_call<int>(platform<void *>(8711376), 76, buffers[0],
                             amount << (Byte(output_channels) & 31), 0, 0);
        platform<Dword>(8711412) = 0;
    }
    platform<Dword>(8711412) = target;
    if (lifecycle<Byte>(8708816))
        service_hardware_streams();
}

void render_output(void *destinations, int count) {
    void *buffers[4];
    for (unsigned c = 0; c < Byte(output_channels); ++c)
        buffers[c] = field<void *>(destinations, c * 4);
    while (count > 0) {
        auto &remaining = platform<int>(8711440);
        if (remaining < 1) {
            ++platform<Dword>(8711444);
            update_audio_voices();
            const Dword sample_product =
                Dword(setting<std::uint16_t>(8708898)) * platform<Dword>(8711444);
            const int sample_position = std::int32_t(sample_product) / 100;
            remaining = int(Dword(sample_position) - platform<Dword>(8711448)) & 268435440;
            platform<Dword>(8711448) += Dword(remaining);
            if (platform<Dword>(8711444) > 30000)
                platform<Dword>(8711444) = platform<Dword>(8711448) = 0;
        }
        int amount = remaining;
        if (count <= remaining)
            amount = count;
        remaining -= amount;
        count -= amount;
        render_mixer(buffers, amount);
        dispatch_stream_events();
        if (!setting<Byte>(8708920))
            buffers[0] =
                static_cast<Byte *>(buffers[0]) + (Dword(amount) << (Byte(output_channels) & 31));
        else
            for (unsigned c = 0; c < Byte(output_channels); ++c)
                buffers[c] = static_cast<Byte *>(buffers[c]) + Dword(amount) * 2;
    }
    if (!lifecycle<Byte>(8708817))
        return;
    if ((++platform_tail<Dword>(8717664) & 3) != 0)
        return;
    using Stopped = int(__cdecl *)(void *);
    for (int i = setting<Byte>(8708902); i < control.voice_count; ++i)
        if (auto stopped = field<Stopped>(backends + i, 256))
            if (stopped(reinterpret_cast<Byte *>(backends + i) + 28))
                stop_voice(i);
}
} // namespace nfs::sound::backend
