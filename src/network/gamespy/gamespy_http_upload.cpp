#include "gamespy_http_runtime.hpp"
namespace nfs::network::session::pending::http {

static Byte send_memory_part(void *item, void *owner) {
    void *source = at<void *>(item, 0);
    const int size = at<int>(source, 12);
    if (!size)
        return 1;
    const int offset = at<int>(item, 4), remaining = size - offset;
    const int sent = send_request(owner, at<char *>(source, 8) + offset, remaining);
    if (sent == -1)
        return 0;
    at<Dword>(item, 4) = Dword(offset) + Dword(sent);
    return sent != remaining ? 2 : 1;
}

Byte pump_post_body(void *owner) {
    const int count = int(dynamic_array_count(at<void *>(owner, 256)));
    if (at<Dword>(owner, 84)) {
        if (!flush_request(buffer(owner, 72), owner))
            return 0;
        if (at<int>(owner, 88) < at<int>(owner, 84))
            return 2;
        clear_buffer(buffer(owner, 72));
        if (at<int>(owner, 260) == count)
            return 1;
    }
    for (; at<int>(owner, 260) < count; ++at<Dword>(owner, 260)) {
        const int index = at<int>(owner, 260);
        void *item = dynamic_array_item(at<void *>(owner, 256), index);
        void *source = at<void *>(item, 0);
        const Dword kind = at<Dword>(source, 0);
        const bool multipart = at<Dword>(at<void *>(owner, 252), 12) != 0;
        if (at<Dword>(item, 4) == Dword(-1)) {
            at<Dword>(item, 4) = 0;
            char header[2048];
            if (!multipart)
                std::sprintf(header, index ? next_url_field_format : first_url_field_format,
                             at<char *>(source, 4));
            else if (kind == 0)
                std::sprintf(header, form_field_format, index ? next_boundary : first_boundary,
                             at<char *>(source, 4));
            else if (kind == 1)
                std::sprintf(header, form_file_format, index ? next_boundary : first_boundary,
                             at<char *>(source, 4), at<char *>(source, 12),
                             at<char *>(source, 16));
            else if (kind == 2)
                std::sprintf(header, form_file_format, index ? next_boundary : first_boundary,
                             at<char *>(source, 4), at<char *>(source, 16),
                             at<char *>(source, 20));
            const Byte state = send_or_queue(owner, header, int(std::strlen(header)));
            if (state != 1)
                return state;
        }
        Byte state;
        if (kind == 0) {
            if (!at<Dword>(source, 12))
                state = 1;
            else if (!multipart && at<Dword>(source, 16)) {
                const auto *value = at<const Byte *>(source, 8);
                auto *out = buffer(owner, 72);
                static constexpr char digits[] = "0123456789ABCDEF";
                for (; *value; ++value)
                    if (std::strchr(url_safe_characters, *value))
                        append_character(out, char(*value));
                    else if (*value == ' ')
                        append_character(out, '+');
                    else {
                        const char encoded[3]{'%', digits[*value >> 4], digits[*value & 15]};
                        append_buffer(out, encoded, 3);
                    }
                state = 1;
            } else {
                const int offset = at<int>(item, 4);
                const int remaining = at<int>(source, 12) - offset;
                // Send the base pointer without adding the retained offset.
                const int sent = send_request(owner, at<char *>(source, 8), remaining);
                if (sent == -1)
                    state = 0;
                else {
                    at<Dword>(item, 4) = Dword(offset) + Dword(sent);
                    state = sent != remaining ? 2 : 1;
                }
            }
        } else if (kind == 1) {
            for (;;) {
                char file_data[4096];
                const int read = int(std::fread(file_data, 1, 4096, at<FILE *>(item, 8)));
                if (read < 1) {
                    fail(owner, 14);
                    state = 0;
                    break;
                }
                at<Dword>(item, 4) += Dword(read);
                if (at<int>(item, 4) > at<int>(item, 12)) {
                    fail(owner, 14);
                    state = 0;
                    break;
                }
                state = send_or_queue(owner, file_data, read);
                if (!state)
                    break;
                if (at<int>(item, 4) == at<int>(item, 12)) {
                    state = 1;
                    break;
                }
                if (state != 1) {
                    state = 2;
                    break;
                }
            }
        } else
            state = send_memory_part(item, owner);
        if (state != 1)
            return state;
    }
    if (at<Dword>(at<void *>(owner, 252), 12))
        if (!send_or_queue(owner, closing_boundary, int(std::strlen(closing_boundary))))
            return 0;
    return at<Dword>(owner, 84) ? 2 : 1;
}
} // namespace nfs::network::session::pending::http
