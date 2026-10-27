#include "nfs/core/memory/memory_primitives.hpp"
#include <cstring>
#include <intrin.h>
#include <mmintrin.h>
#include <xmmintrin.h>
namespace nfs::kernel {
Dword cpu_mmx = 0, cpu_stream_mmx = 0, cpu_sse = 0;
namespace {
using Byte = std::uint8_t;
template <class T> __forceinline T read(const Byte *source) {
    T value;
    std::memcpy(&value, source, sizeof value);
    return value;
}
template <class T> __forceinline void write(Byte *destination, T value) {
    std::memcpy(destination, &value, sizeof value);
}
template <class T>
__forceinline void copy_part(Byte *&destination, const Byte *&source, Dword &bytes) {
    T value = read<T>(source);
    write<T>(destination, value);
    destination += sizeof(T);
    source += sizeof(T);
    bytes -= sizeof(T);
}
__forceinline __m64 load_mmx(const Byte *source) {
    const __m64 value = *reinterpret_cast<const __m64 *>(source);
    _ReadWriteBarrier();
    return value;
}
template <bool streaming> __forceinline void store_mmx(Byte *destination, __m64 value) {
    if constexpr (streaming)
        _mm_stream_pi(reinterpret_cast<__m64 *>(destination), value);
    else {
        *reinterpret_cast<__m64 *>(destination) = value;
        _ReadWriteBarrier();
    }
}
__forceinline void copy_sse_low(Byte *destination, const Byte *source) {
    const __m128 value = _mm_loadl_pi(_mm_setzero_ps(), reinterpret_cast<const __m64 *>(source));
    _mm_storel_pi(reinterpret_cast<__m64 *>(destination), value);
}
template <bool streaming>
__forceinline void copy_32(Byte *&destination, const Byte *&source, Dword &bytes) {
    const __m64 a = load_mmx(source), b = load_mmx(source + 8), c = load_mmx(source + 16),
                d = load_mmx(source + 24);
    store_mmx<streaming>(destination, a);
    store_mmx<streaming>(destination + 8, b);
    store_mmx<streaming>(destination + 16, c);
    store_mmx<streaming>(destination + 24, d);
    destination += 32;
    source += 32;
    bytes -= 32;
}
__forceinline void align_copy(Byte *&destination, const Byte *&source, Dword &bytes, unsigned mask) {
    if (!(reinterpret_cast<Dword>(destination) & mask))
        return;
    if ((reinterpret_cast<Dword>(destination) & 1) && static_cast<std::int32_t>(bytes) >= 1)
        copy_part<Byte>(destination, source, bytes);
    if ((reinterpret_cast<Dword>(destination) & 2) && static_cast<std::int32_t>(bytes) >= 2)
        copy_part<std::uint16_t>(destination, source, bytes);
    if ((reinterpret_cast<Dword>(destination) & 4) && static_cast<std::int32_t>(bytes) >= 4)
        copy_part<Dword>(destination, source, bytes);
    if (mask == 31) {
        if ((reinterpret_cast<Dword>(destination) & 8) && static_cast<std::int32_t>(bytes) >= 8) {
            copy_sse_low(destination, source);
            destination += 8;
            source += 8;
            bytes -= 8;
        }
        if ((reinterpret_cast<Dword>(destination) & 16) && static_cast<std::int32_t>(bytes) >= 16) {
            const __m128 value = _mm_loadu_ps(reinterpret_cast<const float *>(source));
            _mm_store_ps(reinterpret_cast<float *>(destination), value);
            destination += 16;
            source += 16;
            bytes -= 16;
        }
    }
}
__forceinline void copy_tail(Byte *&destination, const Byte *&source, Dword &bytes) {
    if (bytes >= 4)
        copy_part<Dword>(destination, source, bytes);
    if (bytes >= 2)
        copy_part<std::uint16_t>(destination, source, bytes);
    if (bytes >= 1)
        copy_part<Byte>(destination, source, bytes);
}
template <bool streaming>
__forceinline void *buffered_copy(void *destination, const void *source, Dword bytes) {
    auto *output = static_cast<Byte *>(destination);
    auto *input = static_cast<const Byte *>(source);
    align_copy(output, input, bytes, 7);

    if constexpr (streaming)
        while (static_cast<std::int32_t>(bytes - 128) >= 0) {
            const __m64 first = load_mmx(input), second = load_mmx(input + 32),
                        third = load_mmx(input + 64), fourth = load_mmx(input + 96);
            __m64 b = load_mmx(input + 8), c = load_mmx(input + 16), d = load_mmx(input + 24);
            store_mmx<true>(output, first);
            store_mmx<true>(output + 8, b);
            store_mmx<true>(output + 16, c);
            store_mmx<true>(output + 24, d);
            b = load_mmx(input + 40);
            c = load_mmx(input + 48);
            d = load_mmx(input + 56);
            store_mmx<true>(output + 32, second);
            store_mmx<true>(output + 40, b);
            store_mmx<true>(output + 48, c);
            store_mmx<true>(output + 56, d);
            b = load_mmx(input + 72);
            c = load_mmx(input + 80);
            d = load_mmx(input + 88);
            store_mmx<true>(output + 64, third);
            store_mmx<true>(output + 72, b);
            store_mmx<true>(output + 80, c);
            store_mmx<true>(output + 88, d);
            b = load_mmx(input + 104);
            c = load_mmx(input + 112);
            d = load_mmx(input + 120);
            store_mmx<true>(output + 96, fourth);
            store_mmx<true>(output + 104, b);
            store_mmx<true>(output + 112, c);
            store_mmx<true>(output + 120, d);
            output += 128;
            input += 128;
            bytes -= 128;
        }
    while (static_cast<std::int32_t>(bytes - 32) >= 0)
        copy_32<streaming>(output, input, bytes);
    while (static_cast<std::int32_t>(bytes - 8) >= 0) {
        const __m64 value = load_mmx(input);
        store_mmx<streaming>(output, value);
        output += 8;
        input += 8;
        bytes -= 8;
    }
    copy_tail(output, input, bytes);
    return output;
}
__forceinline void fill_part(Byte *&destination, Dword pattern, Dword &bytes, unsigned width) {
    if (width == 1)
        write(destination, static_cast<Byte>(pattern));
    else if (width == 2)
        write(destination, static_cast<std::uint16_t>(pattern));
    else
        for (unsigned i = 0; i < width; i += 4)
            write(destination + i, pattern);
    destination += width;
    bytes -= width;
}
__forceinline void *fill_pattern(void *destination, Dword pattern, Dword bytes) {
    auto *output = static_cast<Byte *>(destination);
    if (reinterpret_cast<Dword>(output) & 3) {
        for (unsigned width = 1; width <= 4; width *= 2)
            if ((reinterpret_cast<Dword>(output) & width) &&
                static_cast<std::int32_t>(bytes) >= static_cast<int>(width))
                fill_part(output, pattern, bytes, width);
    }
    while (static_cast<std::int32_t>(bytes - 32) >= 0)
        fill_part(output, pattern, bytes, 32);
    while (static_cast<std::int32_t>(bytes - 8) >= 0)
        fill_part(output, pattern, bytes, 8);
    if (bytes >= 4)
        fill_part(output, pattern, bytes, 4);
    if (bytes >= 2)
        fill_part(output, pattern, bytes, 2);
    if (bytes >= 1)
        fill_part(output, pattern, bytes, 1);
    return output;
}
template <bool streaming>
__forceinline void *fill_mmx(void *destination, Dword pattern, Dword bytes) {
    auto *output = static_cast<Byte *>(destination);
    if (reinterpret_cast<Dword>(output) & 3) {
        for (unsigned width = 1; width <= 4; width *= 2)
            if ((reinterpret_cast<Dword>(output) & width) &&
                static_cast<std::int32_t>(bytes) >= static_cast<int>(width))
                fill_part(output, pattern, bytes, width);
    }
    const __m64 low = _mm_cvtsi32_si64(static_cast<int>(pattern));
    // Native MOVD/PUNPCKLDQ executes even when no qword will be written.
    volatile __m64 repeated;
    repeated.m64_u64 = _mm_unpacklo_pi32(low, low).m64_u64;
    __m64 value;
    value.m64_u64 = repeated.m64_u64;
    while (static_cast<std::int32_t>(bytes - 32) >= 0) {
        store_mmx<streaming>(output, value);
        store_mmx<streaming>(output + 8, value);
        store_mmx<streaming>(output + 16, value);
        store_mmx<streaming>(output + 24, value);
        output += 32;
        bytes -= 32;
    }
    while (static_cast<std::int32_t>(bytes - 8) >= 0) {
        store_mmx<false>(output, value);
        output += 8;
        bytes -= 8;
    }
    if (bytes >= 4)
        fill_part(output, pattern, bytes, 4);
    if (bytes >= 2)
        fill_part(output, pattern, bytes, 2);
    if (bytes >= 1)
        fill_part(output, pattern, bytes, 1);
    return output;
}
} // namespace

void *__cdecl memory_copy_scalar(void *destination, const void *source, Dword bytes) {
    auto *output = static_cast<Byte *>(destination);
    auto *input = static_cast<const Byte *>(source);
    align_copy(output, input, bytes, 3);
    while (static_cast<std::int32_t>(bytes - 32) >= 0)
        for (int i = 0; i < 8; ++i)
            copy_part<Dword>(output, input, bytes);
    while (static_cast<std::int32_t>(bytes - 8) >= 0) {
        copy_part<Dword>(output, input, bytes);
        copy_part<Dword>(output, input, bytes);
    }
    copy_tail(output, input, bytes);
    return output;
}

__declspec(noinline) void *__cdecl memory_copy_mmx(void *destination, const void *source,
                                                   Dword bytes) {
    return buffered_copy<false>(destination, source, bytes);
}

__declspec(noinline) void *__cdecl memory_copy_sse(void *destination, const void *source,
                                                   Dword bytes) {
    auto *output = static_cast<Byte *>(destination);
    auto *input = static_cast<const Byte *>(source);
    align_copy(output, input, bytes, 31);
    while (static_cast<std::int32_t>(bytes - 32) >= 0) {
        const __m128 first = _mm_loadu_ps(reinterpret_cast<const float *>(input));
        const __m128 second = _mm_loadu_ps(reinterpret_cast<const float *>(input + 16));
        _mm_stream_ps(reinterpret_cast<float *>(output), first);
        _mm_stream_ps(reinterpret_cast<float *>(output + 16), second);
        output += 32;
        input += 32;
        bytes -= 32;
    }
    while (static_cast<std::int32_t>(bytes - 8) >= 0) {
        copy_sse_low(output, input);
        output += 8;
        input += 8;
        bytes -= 8;
    }
    copy_tail(output, input, bytes);
    return output;
}

__declspec(noinline) void *__cdecl memory_copy_stream_mmx(void *destination, const void *source,
                                                          Dword bytes) {
    return buffered_copy<true>(destination, source, bytes);
}

void *__cdecl memory_fill_scalar(void *destination, Dword pattern, Dword bytes) {
    return fill_pattern(destination, pattern, bytes);
}

__declspec(noinline) void *__cdecl memory_fill_mmx(void *destination, Dword pattern, Dword bytes) {
    return fill_mmx<false>(destination, pattern, bytes);
}

__declspec(noinline) void *__cdecl memory_fill_stream_mmx(void *destination, Dword pattern,
                                                          Dword bytes) {
    return fill_mmx<true>(destination, pattern, bytes);
}

__declspec(noinline) void *__cdecl memory_fill_sse(void *destination, Dword pattern, Dword bytes) {
    auto *output = static_cast<Byte *>(destination);
    const __m128 low = _mm_load_ss(reinterpret_cast<const float *>(&pattern)),
                 value = _mm_shuffle_ps(low, low, 0);
    if (reinterpret_cast<Dword>(output) & 31) {
        for (unsigned width = 1; width <= 4; width *= 2)
            if ((reinterpret_cast<Dword>(output) & width) &&
                static_cast<std::int32_t>(bytes) >= static_cast<int>(width))
                fill_part(output, pattern, bytes, width);
        if ((reinterpret_cast<Dword>(output) & 8) && static_cast<std::int32_t>(bytes) >= 8) {
            _mm_storel_pi(reinterpret_cast<__m64 *>(output), value);
            output += 8;
            bytes -= 8;
        }
        if ((reinterpret_cast<Dword>(output) & 16) && static_cast<std::int32_t>(bytes) >= 16) {
            _mm_store_ps(reinterpret_cast<float *>(output), value);
            output += 16;
            bytes -= 16;
        }
    }
    while (static_cast<std::int32_t>(bytes - 32) >= 0) {
        _mm_stream_ps(reinterpret_cast<float *>(output), value);
        _mm_stream_ps(reinterpret_cast<float *>(output + 16), value);
        output += 32;
        bytes -= 32;
    }
    while (static_cast<std::int32_t>(bytes - 8) >= 0) {
        _mm_storel_pi(reinterpret_cast<__m64 *>(output), value);
        output += 8;
        bytes -= 8;
    }
    if (bytes >= 4)
        fill_part(output, pattern, bytes, 4);
    if (bytes >= 2)
        fill_part(output, pattern, bytes, 2);
    if (bytes >= 1)
        fill_part(output, pattern, bytes, 1);
    return output;
}

void *__cdecl memory_copy(void *destination, const void *source, Dword bytes) {
    if (cpu_stream_mmx) {
        void *end = memory_copy_stream_mmx(destination, source, bytes);
        _mm_empty();
        return end;
    }
    if (cpu_sse)
        return memory_copy_sse(destination, source, bytes);
    if (cpu_mmx) {
        void *end = memory_copy_mmx(destination, source, bytes);
        _mm_empty();
        return end;
    }
    return memory_copy_scalar(destination, source, bytes);
}

void *__cdecl memory_zero(void *destination, Dword bytes) {
    if (cpu_sse)
        return memory_fill_sse(destination, 0, bytes);
    if (cpu_stream_mmx) {
        void *end = memory_fill_stream_mmx(destination, 0, bytes);
        _mm_empty();
        return end;
    }
    if (cpu_mmx) {
        void *end = memory_fill_mmx(destination, 0, bytes);
        _mm_empty();
        return end;
    }
    return memory_fill_scalar(destination, 0, bytes);
}

void *__cdecl memory_fill(void *destination, Dword pattern, Dword bytes) {
    if (cpu_stream_mmx) {
        void *end = memory_fill_stream_mmx(destination, pattern, bytes);
        _mm_empty();
        return end;
    }
    if (cpu_sse)
        return memory_fill_sse(destination, pattern, bytes);
    if (cpu_mmx) {
        void *end = memory_fill_mmx(destination, pattern, bytes);
        _mm_empty();
        return end;
    }
    return memory_fill_scalar(destination, pattern, bytes);
}

void __cdecl memory_move(void *destination, const void *source, Dword bytes) {
    Dword output = reinterpret_cast<Dword>(destination), input = reinterpret_cast<Dword>(source);
    if (input < output && output < input + bytes) {
        auto *to = static_cast<Byte *>(destination) + bytes;
        auto *from = static_cast<const Byte *>(source) + bytes;
        while (bytes) {
            *--to = *--from;
            --bytes;
        }
    } else
        memory_copy(destination, source, bytes);
}
} // namespace nfs::kernel
