#include "movie_runtime.hpp"
namespace nfs::movies {
const Dword delegate_methods[1] = {reinterpret_cast<Dword>(delete_delegate)};

static Source *initialize_source(Source *source) {
    source->buffers = 2;
    source->context = nullptr;
    source->acquire = nullptr;
    source->retire = nullptr;
    return source;
}

Source *construct_source(Source *source, Player **context, Acquire acquire, Retire retire,
                      int buffers) {
    source->buffers = buffers;
    source->context = context;
    source->acquire = acquire;
    source->retire = retire;
    return source;
}

Delegate *construct_delegate(Delegate *delegate, const Source *source) {
    // The base constructor targets the embedded record at +8.
    initialize_source(&delegate->source);
    delegate->methods = delegate_methods;
    delegate->source.context = source->context;
    delegate->source.acquire = source->acquire;
    delegate->source.retire = source->retire;
    delegate->source.buffers = source->buffers;
    delegate->decoder = nullptr;
    delegate->chunk = nullptr;
    return delegate;
}

void destroy_delegate(Delegate *delegate) {
    delegate->methods = delegate_methods;
    release_decoder(delegate);
    // A failed first bind can still own a resource chunk that the decoder
    // never consumed. Return that token before the player's pool is destroyed.
    if (delegate->chunk) {
        retire_chunk(delegate, delegate->chunk);
        delegate->chunk = nullptr;
    }
}

void *__fastcall delete_delegate(Delegate *delegate, void *, unsigned flags) {
    destroy_delegate(delegate);
    if (flags & 1)
        memory.release(delegate);
    return delegate;
}

int bind_decoder(Delegate *delegate, void *decoder, Chunk *chunk) {
    delegate->decoder = decoder;
    delegate->chunk = chunk;
    return invoke<int>(decoder, 1, delegate, chunk);
}

void release_decoder(Delegate *delegate) {
    if (delegate->decoder) {
        invoke<void *>(delegate->decoder, 0, 1u);
        delegate->decoder = nullptr;
    }
}

Dword current_frame(Delegate *delegate) {
    return delegate->decoder ? invoke<Dword>(delegate->decoder, 3) : 0;
}

Extended frame_rate(Delegate *delegate) {
    // This virtual method is a FLD/RET float return; retained ST0 transport is an ABI boundary.
     
    return delegate->decoder ? number(invoke<float>(delegate->decoder, 4)) : number(0);
}

void *decode_frame(Delegate *delegate, int frame) {
    if (!delegate->decoder && !delegate->chunk)
        delegate->source.acquire(delegate, delegate->source.context, &delegate->chunk);
    return delegate->decoder ? invoke<void *>(delegate->decoder, 2, frame) : nullptr;
}

void retire_frame(Delegate *delegate, void *frame) {
    invoke<void>(delegate->decoder, 5, frame);
}

Chunk *next_chunk(Delegate *delegate) {
    if (delegate->chunk) {
        Chunk *chunk = delegate->chunk;
        delegate->chunk = nullptr;
        return chunk;
    }
    Chunk *chunk;
    delegate->source.acquire(delegate, delegate->source.context, &chunk);
    return chunk;
}

void retire_chunk(Delegate *delegate, Chunk *chunk) {
    delegate->source.retire(delegate, delegate->source.context, chunk);
}

Chunk *construct_chunk(Chunk *chunk) {
    chunk->data = nullptr;
    chunk->bytes = 0;
    chunk->token = nullptr;
    return chunk;
}
} // namespace nfs::movies
