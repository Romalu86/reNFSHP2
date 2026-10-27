#include "nfs/audio/streams/audio_streams.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::pending {
extern Byte &backend_count;
void voice_range(Byte, int *, int *);
std::uint16_t next_layer_format(int *, void *);
int allocate_layers(Dword, Dword, int *, int, int);
} // namespace nfs::sound::pending
namespace nfs::sound::streaming {
// The first and fourth arguments are not read.
int start_stream_backend(int, int voice, std::uint16_t value, Dword, Dword seek, Dword loop,
                         const Format *, void *const *effects) {
    Dword sources[4] = {};
    field<std::uint16_t>(control.voices + voice, 94) = value;
    return pending::start_backend(voice, Dword(-1), Dword(-1), 0, 0, seek, loop, sources,
                                           Dword(-1), reinterpret_cast<const Dword *>(effects), 0);
}

int start_stream_voice(int slot, const Format *format, const Parameters *parameters,
                       const VoiceParameters *settings) {
    if (!control.initialized)
        return -10;
    unsigned channels = format->channels;
    alignas(4) Byte layer[160];
    field<Byte>(layer, 3) = format->channels;
    field<std::uint16_t>(layer, 24) = parameters->mode;
    int cursor = 0;
    Dword *queue = backend::pending::stream_nodes[slot];
    queue[1] = 0;
    field<Format>(queue, 56) = *format;
    queue[8] = queue[9] = 0;
    field<std::uint16_t>(queue, 30) = field<std::uint16_t>(queue, 28) = 0;
    field<Byte>(queue, 27) = 1;
    for (unsigned channel = 0; channel < field<Byte>(queue, 58); ++channel) {
        field<std::uint16_t>(queue, 16 + channel * 2) = 0;
        field<std::uint16_t>(queue, 8 + channel * 2) = 0;
        queue[15 + channel] = reinterpret_cast<Dword>(parameters->codec_data[channel]);
    }
    while (std::uint16_t selected = pending::next_layer_format(&cursor, layer)) {
        int first, end, handle;
        pending::voice_range(Byte(selected), &first, &end);
        int index = pending::allocate_layers(channels, 101, &handle, first, end);
        if (index < 0) {
            queue[0] = Dword(-9);
            continue;
        }
        queue[0] = Dword(handle);
        field<Byte>(queue, 26) = Byte(backend::largest_voice_channel(index, format->channels));
        Voice *voice = control.voices + index;
        field<std::uint16_t>(voice, 70) = 0;
        field<std::uint16_t>(voice, 24) = field<std::uint16_t>(settings, 8);
        field<std::uint16_t>(voice, 14) = 65535;
        field<std::uint16_t>(voice, 120) = parameters->pitch;
        field<std::uint16_t>(voice, 124) = field<std::uint16_t>(settings, 12);
        field<Dword>(voice, 44) = 0;
        field<Dword>(voice, 52) = Dword(field<std::int8_t>(settings, 0)) << 16;
        field<Dword>(voice, 56) = 0;
        field<Dword>(voice, 64) = 2147483647;
        field<Dword>(voice, 60) = 8323072;
        field<Byte>(voice, 68) = parameters->volume;
        field<Byte>(voice, 88) = field<Byte>(settings, 1);
        field<Byte>(voice, 80) = 1;
        field<Byte>(voice, 81) = field<Byte>(voice, 82) = 0;
        field<Byte>(voice, 84) = parameters->effect;
        field<Byte>(voice, 83) = field<Byte>(settings, 4);
        field<Byte>(voice, 85) = field<Byte>(settings, 5);
        field<Byte>(voice, 34) = field<Byte>(voice, 35) = 0;
        for (unsigned offset = 96; offset <= 112; offset += 4)
            field<Dword>(voice, offset) = 0;
        field<std::int16_t>(voice, 118) =
            std::int16_t(int(std::int8_t(parameters->pitch_depth)) * 100);
        for (unsigned channel = 0; channel < format->channels; ++channel)
            field<std::int16_t>(voice, 72 + channel * 2) = parameters->channel_pan[channel];
        field<Dword>(voice, 20) = Dword(-1);
        field<Dword>(voice, 16) = 0;
        field<std::uint16_t>(voice, 28) = format->sample_rate;
        field<Byte>(voice, 30) = format->codec;
        field<Byte>(voice, 31) = format->channels;
        field<std::uint16_t>(voice, 32) = selected;
        field<std::uint16_t>(voice, 122) = 0;
        calculate_pitch(index);
        calculate_volume(index);
        calculate_effect(0, index);
        int result = start_stream_backend(
            slot, index, field<std::uint16_t>(settings, 14), field<std::uint16_t>(settings, 18),
            field<std::uint16_t>(settings, 20), field<std::uint16_t>(settings, 22), format,
            parameters->codec_data);
        if (result >= 0)
            break;
        for (unsigned channel = 0; channel < channels; ++channel)
            retire_voice(field<std::int16_t>(voice, 4 + channel * 2));
        queue[0] = Dword(result);
    }
    return int(queue[0]);
}

int set_voice_processor(Dword handle, const Dword *descriptor) {
    if (!control.initialized)
        return -10;
    int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor))
            attach_voice_processor(cursor, descriptor);
    }
    return first;
}

int clear_voice_processor(Dword handle, Dword kind) {
    if (!control.initialized)
        return -10;
    int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor))
            detach_voice_processor(cursor, kind);
    }
    return first;
}

int set_voice_channel_levels(Dword handle, const Byte *levels) {
    if (!control.initialized)
        return -10;
    int first = find_voice(handle);
    if (first >= 0) {
        int cursor = -1;
        while (next_voice(first, &cursor))
            apply_voice_channel_levels(cursor, levels);
    }
    return first;
}

int attach_voice_processor(int index, const Dword *descriptor) {
    Voice *voice = control.voices + index;
    if (!(field<Byte>(voice, 32) & 4))
        return -7;
    for (unsigned channel = 0; channel < field<Byte>(voice, 31); ++channel)
        insert_channel_processor(field<std::int16_t>(voice, 4 + channel * 2), int(channel),
                                 descriptor);
    return 0;
}

int detach_voice_processor(int index, Dword kind) {
    Voice *voice = control.voices + index;
    if (field<Byte>(voice, 32) & 4)
        for (unsigned channel = 0; channel < field<Byte>(voice, 31); ++channel)
            remove_channel_processor(field<std::int16_t>(voice, 4 + channel * 2), kind);
    return 0;
}
// The gain product is stored to float before multiplying each level.
void apply_voice_channel_levels(int index, const Byte *levels) {
    Voice *voice = control.voices + index;
    if (!(field<Byte>(voice, 32) & 4))
        return;
    const int voice_gain = field<volatile std::int8_t>(voice, 83);
    const int group_gain = field<volatile std::int8_t>(voice, 69);
    unsigned channels = field<volatile Byte>(voice, 31);
    volatile float gain =
        float((static_cast<long double>(voice_gain) * group_gain) * 0.00000000189215132451181489159353077411651611328125f);
    if (!channels)
        return;
    unsigned outputs = static_cast<volatile Dword &>(output_channels) & 255;
    for (unsigned channel = 0; channel < channels; ++channel) {
        auto *mixed = backends + field<std::int16_t>(voice, 4 + channel * 2);
        for (unsigned output = 0; output < outputs; ++output) {
            auto value = std::int16_t(Dword(std::int8_t(levels[output])) << 8);
            field<std::int16_t>(mixed, output * 2) = value;
            set_mix_volume(field<std::int16_t>(voice, 4 + channel * 2), int(output),
                           float(static_cast<long double>(value) * gain));
            outputs = static_cast<volatile Dword &>(output_channels) & 255;
        }
        channels = field<volatile Byte>(voice, 31);
        levels += 4;
    }
}

void insert_channel_processor(int index, int channel, const Dword *descriptor) {
    auto *voice = mix_voices + index;
    void *node = allocate(descriptor[1]);
    reinterpret_cast<void(__cdecl *)(void *, Dword, int)>(descriptor[3])(node, descriptor[0],
                                                                        channel);
    field<std::uint16_t>(node, 24) = std::uint16_t(descriptor[2]);
    field<Dword>(node, 0) = descriptor[4];
    field<Dword>(node, 4) = descriptor[5];
    backend::insert_processor(&field<void *>(voice, 48), node);
}
// The saved search dereferences the initial head before checking its successor.
void remove_channel_processor(int index, Dword kind) {
    auto *voice = mix_voices + index;
    void *node = field<void *>(voice, 48);
    do {
        if (field<std::uint16_t>(node, 24) == kind) {
            if (auto destroy = field<void(__cdecl *)(void *)>(node, 4))
                destroy(node);
            backend::remove_processor(&field<void *>(voice, 48), node);
            release(node);
            return;
        }
        node = field<void *>(node, 8);
    } while (node);
}
// Single six-word user-processor descriptor.
int set_processor(Dword handle, const Dword *descriptor) {
    if (!control.initialized)
        return -10;
    Stream *stream = lookup(handle);
    if (!stream)
        return -8;
    if (!stream->processor[2]) {
        for (unsigned i = 0; i < 6; ++i)
            stream->processor[i] = descriptor[i];
        if (stream->voice >= 0)
            set_voice_processor(Dword(stream->voice), stream->processor);
    }
    return 0;
}

int clear_processor(Dword handle, Dword kind) {
    if (!control.initialized)
        return -10;
    Stream *stream = lookup(handle);
    if (!stream)
        return -8;
    if (kind == stream->processor[2]) {
        stream->processor[2] = 0;
        if (stream->voice)
            clear_voice_processor(Dword(stream->voice), kind);
    }
    return 0;
}
} // namespace nfs::sound::streaming
