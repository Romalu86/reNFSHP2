#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend {
namespace {
void copy_decoded_span(void *destination, const void *source, Dword bytes) {
    const Dword whole_words = bytes >> 2;
    for (Dword word = 0; word < whole_words; ++word)
        field<volatile Dword>(destination, word * 4) = field<volatile Dword>(source, word * 4);
    for (Dword tail = whole_words * 4; tail < bytes; ++tail)
        field<volatile Byte>(destination, tail) = field<volatile Byte>(source, tail);
}

void clear_decoded_span(void *destination, Dword bytes) {
    const Dword whole_words = bytes >> 2;
    for (Dword word = 0; word < whole_words; ++word)
        field<volatile Dword>(destination, word * 4) = 0;
    for (Dword tail = whole_words * 4; tail < bytes; ++tail)
        field<volatile Byte>(destination, tail) = 0;
}
} // namespace
// Read both locked spans, preserve ring cursor and end countdown.
int device_refill(void *context, int samples, void *scratch, void *decoded) {
    if (samples <= 0)
        return 1;
    if (field<int>(context, 56) >= 0) {
        field<Dword>(context, 56) -= Dword(samples);
        if (field<int>(context, 56) <= 0)
            return -1;
    }
    void *spans[2];
    Dword lengths[2];
    device_call<int>(field<void *>(context, 60), 44, field<Dword>(context, 52) * 2,
                     Dword(samples) * 2, &spans[0], &lengths[0], &spans[1], &lengths[1], 0);
    for (unsigned part = 0; part < 2; ++part)
        if (spans[part])
            if (void *processor = field<void *>(context, 8)) {
                using Process = int(__cdecl *)(void *, Dword, void *, void *, Byte);
                int result = reinterpret_cast<Process>(field<Dword>(processor, 0))(
                    processor, lengths[part] >> 1, scratch, decoded, field<Byte>(context, 26));
                if (result > 1)
                    copy_decoded_span(spans[part], decoded, lengths[part]);
                else
                    clear_decoded_span(spans[part], lengths[part]);
                if (result < 0 && field<int>(context, 56) < 0)
                    field<Dword>(context, 56) = field<Dword>(context, 48);
            }
    device_call<int>(field<void *>(context, 60), 76, spans[0], lengths[0], spans[1],
                     lengths[1]);
    field<Dword>(context, 52) += Dword(samples);
    if (field<int>(context, 52) >= 16384)
        field<Dword>(context, 52) -= 16384;
    return samples;
}
} // namespace nfs::sound::backend
