#include "movie_mad.hpp"
namespace nfs::movies::mad {

void __fastcall retain(Decoder *decoder, void *, Buffer *entry) {
    detach(&entry->link);
    append(&decoder->busy, entry ? &entry->link : nullptr);
    entry->references = int(Dword(entry->references) + 1);
}

Dword __fastcall frame(Decoder *decoder, void *) {
    return decoder->frame;
}

float __fastcall frame_rate(Decoder *decoder, void *) {
    return decoder->rate;
}

void read_chunk(Decoder *decoder, Chunk **output) {
    *output = next_chunk(decoder->source);
    ++decoder->frame;
}

void release_chunk(Decoder *decoder, Chunk *chunk) {
    if (chunk)
        retire_chunk(decoder->source, chunk);
}

Dword chunk_tag(Chunk *chunk) {
    if (!chunk)
        return 1296122987;
    const auto *data = static_cast<const Byte *>(chunk->data);
    return (Dword(data[0]) << 24) | (Dword(data[1]) << 16) | (Dword(data[2]) << 8) | data[3];
}

Buffer *__fastcall seek_frame(Decoder *decoder, void *, int requested) {
    Chunk *chunk;
    read_chunk(decoder, &chunk);
    if (!decoder->first) {
        int skip = int(Dword(requested) - decoder->frame);
        if (skip > 2) {
            do {
                release_chunk(decoder, chunk);
                read_chunk(decoder, &chunk);
            } while (chunk_tag(chunk) != 1296122987);
        } else if (skip > 0) {
            do {
                if (chunk_tag(chunk) == 1296122981) {
                    release_chunk(decoder, chunk);
                    read_chunk(decoder, &chunk);
                }
            } while (--skip);
        }
    }
    if (chunk) {
        Buffer *output;
        try {
            output = pending::decode_chunk(decoder, chunk);
        } catch (...) {
            release_chunk(decoder, chunk);
            throw;
        }
        release_chunk(decoder, chunk);
        return output;
    }
    if (decoder->reference) {
        invoke<void>(decoder, 5, decoder->reference);
        decoder->reference = nullptr;
    }
    return nullptr;
}

Buffer *reserve_reference(Decoder *decoder) {
    Link *link = decoder->available.next;
    if (link == &decoder->available) {
        kernel::error("MAD_CODEC_INTERNAL::CreateIorP() out of frames did you call "
                      "CODEC::ReleaseFrame( FRAME *Frame )\n");
        return nullptr;
    }
    Buffer *entry = buffer(link);
    detach(link);
    append(&decoder->busy, entry ? &entry->link : nullptr);
    entry->references = 2;
    return entry;
}

Buffer *reserve_frame(Decoder *decoder) {
    Link *link = decoder->available.next;
    if (link == &decoder->available) {
        kernel::error("MAD_CODEC_INTERNAL::CreateB() out of frames did you call "
                      "CODEC::ReleaseFrame( FRAME *Frame )\n");
        return nullptr;
    }
    Buffer *entry = buffer(link);
    detach(link);
    append(&decoder->busy, entry ? &entry->link : nullptr);
    entry->references = 1;
    return entry;
}

void __fastcall release_frame(Decoder *decoder, void *, Buffer *entry) {
    entry->references = int(Dword(entry->references) - 1);
    if (entry->references > 0)
        return;
    detach(&entry->link);
    append(&decoder->available, &entry->link);
}
} // namespace nfs::movies::mad
