#include "nfs/audio/backend/sound_ownership.hpp"
#include <intrin.h>
namespace nfs::sound::backend {

Dword cpuid_available() {
    auto saved = __readeflags();
    __writeeflags(saved ^ 2097152);
    auto changed = __readeflags();
    __writeeflags(saved);
    return Dword(changed ^ saved);
}
// Register-only incidental returns are not consumed.
void read_cpu_words(int *words, int leaf) {
    __cpuid(words, leaf);
}

Dword query_cpu_features() {
    OSVERSIONINFOA version;
    version.dwOSVersionInfoSize = 148;
    GetVersionExA(&version);
    bool windows95 = version.dwPlatformId == 1 && version.dwMinorVersion == 0;
    Dword flags = 1;
    if (cpuid_available()) {
        int words[4];
        read_cpu_words(words, 0);
        if (words[0]) {
            read_cpu_words(words, 1);
            if ((words[3] & 33554432) && !windows95)
                flags = 3;
            read_cpu_words(words, int(2147483649u));
            if (words[3] < 0)
                flags |= 4;
        }
    }
    return flags;
}

int discover_configuration() {
    Dword channel_word;
    int effects;
    int status = probe_device(&channel_word, &effects);
    if (status < 0)
        return status;
    Byte channels = Byte(channel_word);
    setting<Byte>(8708868) = Byte(effects);
    setting<Byte>(8708861) = channels > 4 ? 4 : 2;
    setting<Byte>(8708866) = channels;
    setting<Byte>(8708862) = 128;
    setting<std::uint16_t>(8708856) = 11025;
    setting<std::uint16_t>(8708858) = 48000;
    setting<Byte>(8708860) = 1;
    setting<Byte>(8708920) = 0;
    setting<Byte>(8708918) = Byte(effects);
    setting<Byte>(8708917) = 1;
    setting<Byte>(8708908) = 10;
    setting<Byte>(8708875) = 1;
    setting<std::uint16_t>(8708876) = 36;
    if (channels) {
        setting<Byte>(8708875) = 3;
        setting<std::uint16_t>(8708878) = 145;
        setting<std::uint16_t>(8708880) = 146;
    }
    setting<Byte>(8708933) = 1;
    setting<std::uint16_t>(8708936) = 36;
    setting<Byte>(8708902) = 24;
    setting<Byte>(8708905) = 0;
    pending::backend_stream_slot_count = 16;
    setting<std::uint16_t>(8708898) = 22050;
    setting<Byte>(8708928) = 2;
    setting<Byte>(8708921) = 25;
    const unsigned addresses[] = {8708976, 8708984, 8708986, 8708992, 8708994,
                                  8708996, 8709000, 8709002, 8709004, 8709006,
                                  8709008, 8709016, 8709018, 8709024, 8709026,
                                  8709028, 8709032, 8709034, 8709036, 8709038};
    const std::uint16_t values[] = {0,      16384, 49152, 10922, 32768, 54613, 8192,
                                    24576, 40960, 57344, 0,      8192, 57344, 10922,
                                    32768, 54613, 5461, 27306, 38229, 60074};
    for (unsigned i = 0; i < 20; ++i)
        setting<std::uint16_t>(addresses[i]) = values[i];
    control.unknown_7 = Byte(query_cpu_features());
    return 0;
}

int load_configuration(void *output) {
    if (!lifecycle<Dword>(8708800)) {
        lifecycle<int>(8708804) = discover_configuration();
        setting<std::uint16_t>(8708896) = 16;
        setting<Byte>(8708932) = 90;
        setting<Byte>(8708916) = 1;
        lifecycle<Dword>(8708800) = 1;
        for (unsigned i = 0; i < 38; ++i)
            setting<Dword>(8709064 + i * 4) = setting<Dword>(8708888 + i * 4);
    }
    for (unsigned i = 0; i < 52; ++i)
        field<Dword>(output, i * 4) = setting<Dword>(8708856 + i * 4);
    return lifecycle<int>(8708804);
}

int validate_configuration() {
    set_mixer_mode(setting<Byte>(8708921));
    if (control.initialized) {
        for (unsigned i = 0; i < 38; ++i)
            setting<Dword>(8708888 + i * 4) = setting<Dword>(8709064 + i * 4);
        return 0;
    }
    lifecycle<Byte>(8708815) = lifecycle<Byte>(8708816) = lifecycle<Byte>(8708817) = 0;
    auto &channels = setting<Byte>(8708928);
    if (channels < setting<Byte>(8708860))
        channels = setting<Byte>(8708860);
    if (setting<Byte>(8708861) < channels)
        channels = setting<Byte>(8708861);
    if (!setting<Byte>(8708920) && channels > 2)
        channels = 2;
    auto &rate = setting<std::uint16_t>(8708898);
    if (rate < setting<std::uint16_t>(8708856))
        rate = setting<std::uint16_t>(8708856);
    if (rate > setting<std::uint16_t>(8708858))
        rate = setting<std::uint16_t>(8708858);
    auto &software = setting<Byte>(8708902), &hardware = setting<Byte>(8708905);
    if (software > setting<Byte>(8708862))
        software = setting<Byte>(8708862);
    if (hardware > setting<Byte>(8708866))
        hardware = setting<Byte>(8708866);
    if (pending::backend_stream_slot_count > 32)
        pending::backend_stream_slot_count = 32;
    if (setting<Byte>(8708918) && !setting<Byte>(8708868))
        setting<Byte>(8708918) = 0;
    if (setting<Byte>(8708920) && setting<Byte>(8708866) < 5)
        setting<Byte>(8708920) = 0;
    control.voice_count = std::int16_t(int(software) + int(hardware));
    if (control.voice_count > 256) {
        hardware = Byte(hardware - control.voice_count);
        control.voice_count = 256;
    }
    for (;;) {
        unsigned index = 0;
        auto &formats = setting<Byte>(8708933);
        for (; index < formats; ++index) {
            auto &format = setting<std::uint16_t>(8708936 + index * 2);
            if (!hardware && (format & 16))
                format = 0;
            if (format & 16) {
                lifecycle<Byte>(8708815) = 1;
                if (format & 1)
                    lifecycle<Byte>(8708817) = 1;
                else if (format & 2)
                    lifecycle<Byte>(8708816) = 1;
            }
            if (!format)
                break;
        }
        if (index == formats)
            break;
        --formats;
        for (; index < formats; ++index)
            setting<std::uint16_t>(8708936 + index * 2) =
                setting<std::uint16_t>(8708938 + index * 2);
    }
    if (!lifecycle<Byte>(8708815))
        hardware = 0;
    auto &period = setting<Byte>(8708908);
    if (!period)
        period = 1;
    else if (period > 200)
        period = 200;
    if (setting<Byte>(8708917) && (query_cpu_features() & 2)) {
        control.unknown_7 |= 2;
        return 0;
    }
    setting<Byte>(8708917) = 0;
    control.unknown_7 &= 253;
    return 0;
}

int apply_configuration(const void *input) {
    for (unsigned i = 0; i < 38; ++i)
        setting<Dword>(8708888 + i * 4) = field<Dword>(input, 32 + i * 4);
    for (unsigned i = 0; i < 6; ++i)
        setting<Dword>(8709040 + i * 4) = field<Dword>(input, 184 + i * 4);
    validate_configuration();
    for (unsigned i = 0; i < 38; ++i)
        setting<Dword>(8709064 + i * 4) = setting<Dword>(8708888 + i * 4);
    return 0;
}

int pan_curve(int value) {
    return int(Dword(value) * 2) - int(Dword(value) * Dword(value)) / 127;
}

void build_pan_table() {
    unsigned channels = Byte(output_channels);
    auto boundary = [&](unsigned address) { return int(setting<std::uint16_t>(address)); };
    for (int bin = 0; bin < 256; ++bin) {
        int angle = bin * 256;
        auto *pan = pending::pan_table + bin * 4;
        if (channels == 1) {
            pan[0] = 127;
            continue;
        }
        for (unsigned c = 0; c < channels; ++c)
            pan[c] = 0;
        int first = boundary(8708968 + channels * 8), last = boundary(8708966 + channels * 10),
            left, right;
        if (first < angle && angle < last) {
            for (unsigned c = 0; c < channels; ++c) {
                int a = boundary(8708968 + channels * 8 + c * 2),
                    b = boundary(8708970 + channels * 8 + c * 2);
                if (a <= angle && angle <= b) {
                    left = b - angle;
                    right = angle - a;
                    pan[c] = std::int8_t(pan_curve(left * 127 / (left + right)));
                    pan[c + 1] = std::int8_t(pan_curve(right * 127 / (left + right)));
                }
            }
            continue;
        }
        if (first < angle) {
            left = first - angle + 65536;
            right = angle - last;
        } else {
            left = first - angle;
            right = angle - last + 65536;
        }
        pan[0] = std::int8_t(pan_curve(right * 127 / (left + right)));
        pan[channels - 1] = std::int8_t(pan_curve(left * 127 / (left + right)));
    }
}
} // namespace nfs::sound::backend
