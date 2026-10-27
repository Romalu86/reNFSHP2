#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend::pending {
extern int process_rate(void *, int, void *, void *, Byte);
using Interpolate = void(__cdecl *)(Dword, const void *, void *, int *, Dword *, int, Dword);
extern void cubic_scalar(Dword, const void *, void *, int *, Dword *, int, Dword);
extern void cubic_packed(Dword, const void *, void *, int *, Dword *, int, Dword);
extern void cubic_streaming(Dword, const void *, void *, int *, Dword *, int, Dword);
extern void linear_scalar(Dword, const void *, void *, int *, Dword *, int, Dword);
extern void linear_packed(Dword, const void *, void *, int *, Dword *, int, Dword);
extern void linear_streaming(Dword, const void *, void *, int *, Dword *, int, Dword);
// Keep the six real CPU interpolation choices.
void construct_rate(void *processor, Byte features, int cubic) {
    field<Dword>(processor, 4) = field<Dword>(processor, 32) = 0;
    field<std::uint16_t>(processor, 40) = field<std::uint16_t>(processor, 42) = 0;
    field<Dword>(processor, 0) = reinterpret_cast<Dword>(process_rate);
    Interpolate method;
    if (cubic) {
        field<std::uint16_t>(processor, 42) = 1;
        method = features & 2   ? cubic_streaming
                 : features & 4 ? cubic_packed
                                : cubic_scalar;
    } else
        method = features & 2   ? linear_streaming
                 : features & 4 ? linear_packed
                                : linear_scalar;
    field<Dword>(processor, 36) = reinterpret_cast<Dword>(method);
    for (unsigned offset = 44; offset <= 56; offset += 4)
        field<Dword>(processor, offset) = 0;
}
} // namespace nfs::sound::backend::pending
