#include "nfs/physics/frame/dynamics_runtime.hpp"
#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend {
namespace {
int codec_index(int format, bool mixer) {
    switch (format) {
    case 7:
    case 8:
        return 3;
    case 9:
        return 0;
    case 10:
        return 6;
    case 4:
        return 9;
    case 14:
        return mixer ? 12 : 0;
    case 15:
        return 15;
    case 16:
        return 18;
    case 64:
        return 21;
    case 20:
        return mixer ? 24 : 0;
    default:
        return 0;
    }
}
} // namespace
// Constructor tables remain real registered providers, never null substitutes.
void *construct_codec(Dword data, int format, Dword length, int loop, Dword channels, Dword source,
                      int voice, Dword *position) {
    int index = codec_index(format, false);
    if (!data)
        index += 2;
    else if (loop > 0)
        ++index;
    void *processor = nullptr;
    if (pending::codec_constructors[index]) {
        processor = allocate(pending::codec_sizes[index]);
        field<Byte>(processor, 26) = 0;
        field<Dword>(processor, 4) = 0;
        Dword parameters[10]; // +4 is unwritten in this constructor.
        parameters[0] = data;
        parameters[2] = channels;
        parameters[3] = source;
        parameters[4] = length;
        parameters[5] = Dword(loop);
        parameters[6] = Dword(format == 9 ? 0 : 1);
        parameters[7] = Dword(voice);
        parameters[8] = control.unknown_7;
        parameters[9] = 0;
        using Construct = void(__cdecl *)(void *, Dword *);
        reinterpret_cast<Construct>(pending::codec_constructors[index])(processor,
                                                                                 parameters);
        *position = parameters[9];
    }
    return processor;
}

void start_mix_voice(int voice, int format, int memory, Dword data, Dword effect, int resample,
                     Dword channels, Dword source, Dword length, int loop, Dword, Dword, Byte channel) {
    auto *mix = mix_voices + voice;
    for (unsigned offset : {48u, 56u, 64u, 72u, 76u, 60u})
        field<Dword>(mix, offset) = 0;
    int index = codec_index(format, true);
    if (memory == 1) {
        if (loop > 0)
            ++index;
    } else if (!memory)
        index += 2;
    if (pending::codec_constructors[index]) {
        void *processor = allocate(pending::codec_sizes[index]);
        field<void *>(mix, 56) = processor;
        Dword parameters[10] = {data,
                               effect,
                               channels,
                               source,
                               length,
                               Dword(loop),
                               Dword(format == 9 ? 0 : 1),
                               Dword(voice),
                               control.unknown_7,
                               0};
        field<Dword>(processor, 4) = 0;
        field<std::uint16_t>(processor, 24) = 240;
        field<Byte>(processor, 26) = channel;
        using Construct = void(__cdecl *)(void *, Dword *);
        reinterpret_cast<Construct>(pending::codec_constructors[index])(processor,
                                                                                 parameters);
        field<Dword>(mix, 52) = parameters[9];
        insert_processor(&field<void *>(mix, 48), processor);
    }
    if (resample) {
        int owner = memory ? -1 : voice;
        void *processor = allocate(6184);
        field<void *>(mix, 60) = processor;
        field<Dword>(processor, 4) = 0;
        field<std::uint16_t>(processor, 24) = 200;
        construct_resampler(processor, reinterpret_cast<void *>(resample), owner);
        insert_processor(&field<void *>(mix, 48), processor);
    }
    field<Byte>(mix, 0) = 1;
}

void construct_converter(void *processor) {
    field<Dword>(processor, 0) = reinterpret_cast<Dword>(pending::process_float);
    field<Dword>(processor, 4) = 0;
}

int construct_resampler(void *processor, const void *data, int voice) {
    field<Dword>(processor, 0) = reinterpret_cast<Dword>(pending::process_resampler);
    field<Dword>(processor, 4) = 0;
    field<int>(processor, 32) = voice < 0 ? -1 : stream_slot(root_voice(voice));
    field<Dword>(processor, 36) = 1065353216;
    field<Dword>(processor, 40) = 0;
    field<const void *>(processor, 28) = static_cast<const Byte *>(data) + 6;
    field<Dword>(processor, 44) = field<Byte>(data, 1) * 2;
    field<int>(processor, 48) = read_bank_integer(static_cast<const Byte *>(data) + 2, 4);
    field<Dword>(processor, 52) = field<Dword>(processor, 56) = field<Dword>(processor, 60) = 0;
    return 0;
}

int construct_buffer_chain(void *context, const void *parameters) {
    const int voice = field<std::int16_t>(parameters, 2);
    field<std::int16_t>(context, 2) = std::int16_t(voice);
    auto *owner = backends + voice;
    for (unsigned offset : {8u, 12u, 16u, 212u, 20u, 24u})
        field<Dword>(owner, offset) = 0;
    auto **chain = &field<void *>(owner, 212);
    void *codec = construct_codec(field<Dword>(parameters, 20), field<Byte>(parameters, 4),
                                  field<Dword>(parameters, 8), field<int>(parameters, 12),
                                  field<Byte>(parameters, 5), field<Dword>(parameters, 16), voice,
                                  &field<Dword>(context, 4));
    field<void *>(owner, 8) = codec;
    field<std::uint16_t>(codec, 24) = 5;
    insert_processor(chain, codec);
    void *convert = allocate(28);
    field<void *>(owner, 12) = convert;
    construct_converter(convert);
    field<std::uint16_t>(convert, 24) = 1;
    insert_processor(chain, convert);
    if (field<Dword>(parameters, 24)) {
        int index = field<Dword>(parameters, 16) ? -1 : voice;
        void *resampler = allocate(6184);
        field<void *>(owner, 16) = resampler;
        construct_resampler(resampler, field<void *>(parameters, 24), index);
        field<std::uint16_t>(resampler, 24) = 4;
        insert_processor(chain, resampler);
    }
    return 1;
}
} // namespace nfs::sound::backend
