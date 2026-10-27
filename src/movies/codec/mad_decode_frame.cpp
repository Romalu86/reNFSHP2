#include "mad_codec.hpp"
#include "../playback/movie_clock_x87.hpp"
#include <new>
#include <stdexcept>
namespace nfs::movies::mad::pending {
namespace {
class FrameControl {
public:
    FrameControl() : saved(codec::change_fp_control(0, 0)) {
        codec::change_fp_control(65536, 196608);
    }
    ~FrameControl() { restore(); }
    void restore() {
        if (active) {
            codec::change_fp_control(saved, 1048575);
            active = false;
        }
    }
private:
    Dword saved;
    bool active = true;
};
}

Buffer *decode_chunk(Decoder *decoder, Chunk *chunk) {
    namespace data = codec;
    if (chunk->bytes < 24)
        throw std::runtime_error("MAD frame header is truncated");
    Dword compressed = data::address(chunk->data);
    const Dword tag_at_entry = chunk_tag(chunk);
    const Dword header_width = data::load<std::uint16_t>(data::address(compressed + 16));
    const Dword header_height = data::load<std::uint16_t>(data::address(compressed + 18));
    const Dword duration_at_entry = data::load<Dword>(data::address(compressed + 12));
    const std::uint64_t image_bytes = std::uint64_t(header_width) * header_height * 3 / 2 + 16;
    if (!header_width || !header_height || (header_width & 15) || (header_height & 15) ||
        int(duration_at_entry) <= 0 || image_bytes > 2147483647u)
        throw std::runtime_error("MAD frame dimensions or duration cannot be decoded safely");
    if ((decoder->first && tag_at_entry != 1296122987) ||
        (!decoder->first && (header_width != decoder->width || header_height != decoder->height)))
        throw std::runtime_error("MAD frame does not match its decoder reference geometry");
    Dword reference_y = 0, reference_u = 0, reference_v = 0;
    auto pixels = [](void *shape) {
        Dword base = data::address(shape);
        return base + ((data::load<Dword>(data::address(base + 12)) & 4096)
                           ? data::load<Dword>(data::address(base + 16))
                           : 16);
    };
    if (decoder->reference) {
        reference_y = pixels(decoder->reference->shape);
        Dword count = decoder->height * decoder->width;
        reference_u = reference_y + count;
        reference_v = reference_y + Dword(int(count * 5) / 4);
    }
    if (decoder->first) {
        Dword duration = data::load<Dword>(data::address(compressed + 12));
        Delegate *source = decoder->source;
        decoder->duration = duration;
        clock_native::initialize_frame_rate(decoder, std::int32_t(duration));
        decoder->width = data::load<std::uint16_t>(data::address(compressed + 16));
        decoder->height = data::load<std::uint16_t>(data::address(compressed + 18));
        int count = int(Dword(source->source.buffers) + 1);
        if (count > 0)
            do {
                Dword height = decoder->height, width = decoder->width;
                auto *storage = static_cast<Buffer *>(memory.allocate("", 20, 0, 0, memory.pool));
                if (!storage)
                    throw std::bad_alloc();
                Buffer *entry;
                try {
                    entry = construct_buffer(storage, width, height);
                } catch (...) {
                    memory.release(storage);
                    throw;
                }
                Link *link = entry ? &entry->link : nullptr;
                Link *first = decoder->available.next;
                decoder->available.next = link;
                first->previous = link;
                link->previous = &decoder->available;
                link->next = first;
            } while (--count);
        decoder->first = 0;
    }
    FrameControl control;
    Dword tag = chunk_tag(chunk);
    Buffer *output;
    Dword use_reference;
    int quantizer;
    if (tag == 1296122987) {
        if (decoder->reference)
            invoke<void>(decoder, 5, decoder->reference);
        decoder->reference = nullptr;
        output = reserve_reference(decoder);
        quantizer = data::load<std::int8_t>(data::address(compressed + 21));
        use_reference = 0;
    } else {
        tag = chunk_tag(chunk);
        if (tag == 1296122989)
            output = reserve_reference(decoder);
        else {
            chunk_tag(chunk);
            output = reserve_frame(decoder);
        }
        quantizer = data::load<std::int8_t>(data::address(compressed + 21));
        use_reference = 1;
    }
    data::prepare_frame(data::address(compressed + 24), use_reference, quantizer);
    Dword output_y = pixels(output->shape);
    Dword stride = decoder->width;
    Dword count = stride * decoder->height;
    Dword output_u = output_y + count, output_v = output_y + Dword(int(count * 5) / 4);
    for (int row = 0; row < int(decoder->height); row += 16) {
        for (int column = 0; column < int(stride); column += 16) {
            Dword luma_offset = stride * Dword(row);
            Dword chroma_offset = Dword(int(luma_offset) / 4) + Dword(column / 2);
            luma_offset += Dword(column);
            data::decode_macroblock(reference_y + luma_offset, reference_u + chroma_offset,
                                    reference_v + chroma_offset, output_y + luma_offset,
                                    output_u + chroma_offset, output_v + chroma_offset, stride);
            stride = decoder->width;
        }
    }
    control.restore();
    tag = chunk_tag(chunk);
    if (tag == 1296122989) {
        if (decoder->reference)
            invoke<void>(decoder, 5, decoder->reference);
        decoder->reference = output;
    } else if (chunk_tag(chunk) == 1296122987)
        decoder->reference = output;
    return output;
}
} // namespace nfs::movies::mad::pending
