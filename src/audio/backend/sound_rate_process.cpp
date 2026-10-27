#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend::pending {
using Process = int(__cdecl *)(void *, int, void *, void *, Byte);
using Interpolate = void(__cdecl *)(Dword, const void *, void *, int *, Dword *, int, Dword);
// Four saved history samples straddle each source request.
int process_rate(void *state, int count, void *input, void *output, Byte) {
    auto *source = static_cast<Dword *>(input), *out = static_cast<Dword *>(output);
    const Dword rate = field<Dword>(state, 28);
    auto &started = field<std::int16_t>(state, 40);
    auto clear_history = [&]() {
        if (!started)
            started = 1;
        for (unsigned k = 0; k < 4; ++k)
            field<Dword>(state, 44 + k * 4) = 0;
    };
    if (rate != 65536) {
        Dword required = (rate * Dword(count) + field<Dword>(state, 32)) >> 16;
        if (!started) {
            required += Dword(4 - field<std::int16_t>(state, 42));
            for (int k = 0; k < field<std::int16_t>(state, 42); ++k)
                *source++ = 0;
        } else {
            for (unsigned k = 0; k < 4; ++k)
                source[k] = field<Dword>(state, 44 + k * 4);
            source += 4;
        }
        if (int(required) > 0) {
            if (void *upstream = field<void *>(state, 8)) {
                int got = field<Process>(upstream, 0)(upstream, int(required), output, source,
                                                      field<Byte>(state, 26));
                if (got < 1) {
                    clear_history();
                    return got;
                }
            }
        }
        if (!started)
            started = 1;
        else
            source += field<std::int16_t>(state, 42) - 4;
        Dword phase = field<Dword>(state, 32) << 16;
        int index = 0;
        field<Interpolate>(state, 36)(Dword(count), source, out, &index, &phase,
                                        field<std::uint16_t>(state, 30),
                                        field<Dword>(state, 28) << 16);
        field<Dword>(state, 32) = (Dword(index) << 16) | (phase >> 16);
        int history = field<std::uint16_t>(state, 34) - field<std::int16_t>(state, 42);
        for (unsigned k = 0; k < 4; ++k)
            field<Dword>(state, 44 + k * 4) = source[history + k];
        field<std::uint16_t>(state, 34) = 0;
        return count;
    }
    int required = count;
    if (!started)
        required = count - field<std::int16_t>(state, 42) + 4;
    else
        for (int k = 0; k < 4 - field<std::int16_t>(state, 42); ++k)
            *out++ = field<Dword>(state, 44 + (field<std::int16_t>(state, 42) + k) * 4);
    void *upstream = field<void *>(state, 8);
    int got = upstream ? field<Process>(upstream, 0)(upstream, required, source, out,
                                                     field<Byte>(state, 26))
                       : count;
    if (got < 1) {
        clear_history();
        return got;
    }
    int offset;
    if (!started) {
        started = 1;
        offset = count - field<std::int16_t>(state, 42);
    } else
        offset = count - 4;
    out += offset;
    for (unsigned k = 0; k < 4; ++k)
        field<Dword>(state, 44 + k * 4) = out[k];
    return count;
}
} // namespace nfs::sound::backend::pending
