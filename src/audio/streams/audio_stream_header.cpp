#include "nfs/audio/streams/audio_streams.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::sound::streaming {
// Reserved words remain unwritten, as in the initializer.
int initialize_stream_parameters(Parameters *parameters) {
    parameters->group = 0;
    parameters->pitch = 0;
    parameters->volume = 127;
    parameters->pan = 64;
    parameters->effect = parameters->pitch_depth = parameters->flags = 0;
    parameters->mode = 0;
    for (unsigned i = 0; i < 4; ++i) {
        parameters->envelopes[i] = nullptr;
        parameters->envelope_bytes[i] = 0;
    }
    for (unsigned i = 0; i < 4; ++i) {
        parameters->channel_pan[i] = 0;
        parameters->codec_data[i] = nullptr;
    }
    return 0;
}

void decode_header(Dword base, const void *data, Format *format, Parameters *parameters,
                   Metadata *metadata) {
    Dword pan[4] = {}, mode = 0, legacy_mode = 0;
    zero_memory(format, 4);
    zero_memory(parameters, 84);
    zero_memory(metadata, 20);
    initialize_stream_parameters(parameters);
    format->sample_rate = 22050;
    format->channels = 1;
    format->codec = 10;
    metadata->samples = 0;
    BankField attribute;
    attribute.cursor = static_cast<Byte *>(const_cast<void *>(data)) + 4;
    unsigned envelope = 0;
    while (next_bank_field(&attribute)) {
        Dword tag = attribute.tag;
        if (tag == 129 || tag == 131)
            continue;
        if (tag == 160)
            format->codec = Byte(attribute.value);
        else if (tag >= 156 && tag <= 159)
            pan[tag - 156] = attribute.value;
        else if (tag >= 152 && tag <= 155) {
            unsigned channel = tag - 152;
            void *copy = allocate(attribute.length);
            parameters->codec_data[channel] = copy;
            kernel::memory_copy(copy, attribute.payload, attribute.length);
            parameters->codec_bytes[channel] = attribute.length;
        } else if (tag == 128)
            parameters->flags = Byte(attribute.value);
        else if (tag == 130)
            format->channels = Byte(attribute.value);
        else if (tag == 132)
            format->sample_rate = std::uint16_t(attribute.value);
        else if (tag == 133)
            metadata->samples = attribute.value;
        else if (tag == 138)
            continue;
        else if (tag == 19)
            parameters->effect = Byte(attribute.value);
        else if (tag == 10)
            parameters->pitch_depth = Byte(attribute.value);
        else if (tag == 6)
            parameters->group = Byte(attribute.value);
        else if (tag == 140)
            mode = attribute.value;
        else if (tag == 136)
            metadata->channel_offsets[0] = attribute.value;
        else if (tag == 137)
            metadata->channel_offsets[1] = attribute.value;
        else if (tag == 148)
            metadata->channel_offsets[2] = attribute.value;
        else if (tag == 149)
            metadata->channel_offsets[3] = attribute.value;
        else if (tag == 161)
            legacy_mode = attribute.value;
        else if (tag == 20) {
            parameters->envelopes[envelope] = attribute.payload;
            parameters->envelope_bytes[envelope] = attribute.length;
            ++envelope;
        }
    }
    if (legacy_mode && !mode) {
        mode = (legacy_mode & 1) ? 1 : 0;
        if (legacy_mode & 2)
            mode |= 2;
        if (std::int8_t(legacy_mode) < 0)
            mode |= 32;
        if (legacy_mode & 64)
            mode |= 64;
    }
    parameters->mode = std::uint16_t(mode);
    for (int channel = int(format->channels) - 1; channel >= 0; --channel) {
        if (Byte(output_channels) == 2 && format->channels == 2)
            pan[channel] = 0;
        auto routing = field<std::int16_t>(effect_groups,
                                           36 + (channel + unsigned(format->channels) * 4) * 2);
        parameters->channel_pan[channel] =
            std::int16_t(Dword(routing) + std::uint16_t(pan[channel]));
        if ((parameters->mode & 20) || !parameters->mode)
            metadata->channel_offsets[channel] += base;
    }
}
} // namespace nfs::sound::streaming
