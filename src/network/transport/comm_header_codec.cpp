#include "comm_filter_runtime.hpp"
#include <intrin.h>
#include <mmsystem.h>
namespace nfs::network::session::pending {

Dword comm_milliseconds() {
    if (kernel::timer_frequency)
        return (kernel::timer_count() * 1000) / kernel::timer_frequency;
    return (timeGetTime() * 1000) / 1000;
}
// C++ compiler intrinsic for the saved RDTSC instruction.
Dword comm_timestamp() {
    return Dword(__rdtsc());
}

static const char *skip_format_digits(const char *format) {
    if (format)
        while (*format && *format > '/' && *format < ':')
            ++format;
    return format;
}

static Dword processor_byte_order(Dword processor) {
    switch (processor) {
    case 4:
    case 5:
    case 6:
    case 7:
        return 1;
    default:
        return 0;
    }
}
// Complete constant-returning leaf.
static Dword comm_processor_kind() {
    return 1;
}

static Dword host_byte_order() {
    return processor_byte_order(comm_processor_kind());
}

static void *convert_words(Dword from, Dword to, void *data, Dword count) {
    if (data && count && from != to)
        for (Dword i = 0; i < count; ++i) {
            Byte *word = bytes(data) + i * 4;
            const Dword first = at<volatile Byte>(word, 0);
            const Dword third = at<volatile Byte>(word, 2);
            const Dword second = at<volatile Byte>(word, 1);
            const Dword fourth = at<volatile Byte>(word, 3);
            at<volatile Dword>(word, 0) =
                (first << 24) | (second << 16) | (third << 8) | fourth;
        }
    return data;
}

static void *convert_shorts(Dword from, Dword to, void *data, Dword count) {
    if (data && count && from != to)
        for (Dword i = 0; i < count; ++i) {
            Byte *word = bytes(data) + i * 2;
            const unsigned first = at<volatile Byte>(word, 0);
            const unsigned second = at<volatile Byte>(word, 1);
            at<volatile std::uint16_t>(word, 0) =
                std::uint16_t((first << 8) | second);
        }
    return data;
}

static void *convert_header(Dword from, Dword to, void *data, const char *format) {
    if (data && format && from != to)
        while (*format) {
            switch (*format) {
            case 'C':
            case 'c': {
                const int count = std::atoi(++format);
                data = bytes(data) + count;
                break;
            }
            case 'I':
            case 'i': {
                const int count = std::atoi(++format);
                if (count > 0) {
                    convert_words(from, to, data, Dword(count));
                    data = bytes(data) + Dword(count) * 4;
                }
                break;
            }
            case 'S':
            case 's': {
                const int count = std::atoi(++format);
                if (count > 0) {
                    convert_shorts(from, to, data, Dword(count));
                    data = bytes(data) + Dword(count) * 2;
                }
                break;
            }
            }
            format = skip_format_digits(format);
        }
    return data;
}

void encode_comm_header(void *data, const char *format) {
    Dword peer = 0;
    if (Dword *options = comm_options())
        peer = options[1];
    convert_header(host_byte_order(), peer, data, format);
}

void decode_comm_header(void *data, const char *format) {
    Dword peer = 0;
    if (Dword *options = comm_options())
        peer = options[1];
    convert_header(peer, host_byte_order(), data, format);
}
} // namespace nfs::network::session::pending
