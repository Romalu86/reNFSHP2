#include "nfs/core/errors/assertions.hpp"
#include "movie_mad.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include <new>
namespace nfs::movies::mad {
const Dword methods[6] = {
    reinterpret_cast<Dword>(delete_decoder), reinterpret_cast<Dword>(initialize),
    reinterpret_cast<Dword>(seek_frame),     reinterpret_cast<Dword>(frame),
    reinterpret_cast<Dword>(frame_rate),     reinterpret_cast<Dword>(release_frame)};
const Dword base_methods[6] = {
    reinterpret_cast<Dword>(delete_base), reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),   reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),   reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call)};

Buffer *construct_buffer(Buffer *buffer, Dword width, Dword height) {
    buffer->references = 0;
    buffer->kind = 1;
    Dword bytes = ((width * height * 3) >> 1) + 16;
    buffer->shape = memory.allocate("MAD_FRAME::m_Shape", int(bytes), 0, 16, memory.pool);
    if (!buffer->shape)
        throw std::bad_alloc();
    kernel::memory_fill(buffer->shape, 0, 16);
    field<Byte>(buffer->shape, 0) = 108;
    field<std::uint16_t>(buffer->shape, 4) = std::uint16_t(width);
    field<std::uint16_t>(buffer->shape, 6) = std::uint16_t(height);
    return buffer;
}

void destroy_buffer(Buffer *buffer) {
    if (buffer->shape) {
        memory.release(buffer->shape);
        buffer->shape = nullptr;
    }
}

Decoder *construct(Decoder *decoder) {
    decoder->busy.next = decoder->busy.previous = &decoder->busy;
    decoder->available.next = decoder->available.previous = &decoder->available;
    decoder->methods = methods;
    decoder->first = 1;
    return decoder;
}

void destroy(Decoder *decoder) {
    decoder->methods = methods;
    auto clear = [](Link *head) {
        while (head->next != head) {
            Link *link = head->next;
            Buffer *entry = buffer(link);
            detach(link);
            if (entry) {
                destroy_buffer(entry);
                memory.release(entry);
            }
        }
    };
    clear(&decoder->busy);
    clear(&decoder->available);
    // The base member destructors visit both heads again, in reverse order.
    clear(&decoder->available);
    clear(&decoder->busy);
    decoder->methods = base_methods;
}

void *__fastcall delete_decoder(Decoder *decoder, void *, unsigned flags) {
    destroy(decoder);
    if (flags & 1)
        memory.release(decoder);
    return decoder;
}

void *__fastcall delete_base(Decoder *decoder, void *, unsigned flags) {
    decoder->methods = base_methods;
    if (flags & 1)
        memory.release(decoder);
    return decoder;
}
// Second stack word is ignored; RET8 is retained.
int __fastcall initialize(Decoder *decoder, void *, Delegate *source, Chunk *) {
    decoder->frame = 0;
    decoder->reference = nullptr;
    decoder->source = source;
    return 2;
}
} // namespace nfs::movies::mad
namespace nfs::movies::pending {

void *create_mad_decoder() {
    auto *decoder = static_cast<mad::Decoder *>(memory.allocate("", 52, 0, 0, memory.pool));
    return decoder ? mad::construct(decoder) : nullptr;
}
} // namespace nfs::movies::pending
