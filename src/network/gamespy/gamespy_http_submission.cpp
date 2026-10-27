#include "gamespy_http_runtime.hpp"
#include "gamespy_browser.hpp"
namespace nfs::network::authentication::pending {
char *duplicate(const char *);
}
namespace nfs::network::session::pending::http {
namespace {
Dword startup_count{};
Dword multipart_tail_bytes{}, multipart_file_bytes{}, multipart_field_bytes{},
    multipart_boundary_bytes{};
} // namespace

void *allocate_dynamic_array(int stride, int capacity, void(__cdecl *destroy)(void *)) {
    auto *array = static_cast<Dword *>(std::malloc(24));
    if (!capacity)
        capacity = 8;
    array[2] = Dword(stride);
    array[0] = 0;
    array[1] = array[3] = Dword(capacity);
    array[4] = reinterpret_cast<Dword>(destroy);
    array[5] = reinterpret_cast<Dword>(std::malloc(Dword(stride) * Dword(capacity)));
    return array;
}

void append_dynamic_array(void *array, const void *value) {
    const Dword index = at<Dword>(array, 0);
    if (index == at<Dword>(array, 4)) {
        at<Dword>(array, 4) += at<Dword>(array, 12);
        at<void *>(array, 20) =
            std::realloc(at<void *>(array, 20), at<Dword>(array, 8) * at<Dword>(array, 4));
    }
    at<Dword>(array, 0) = index + 1;
    std::memcpy(bytes(at<void *>(array, 20)) + at<Dword>(array, 8) * index, value,
                at<Dword>(array, 8));
}

static Dword prepare_post_body(void *owner) {
    void *post = at<void *>(owner, 252);
    if (!post)
        return 0;
    at<Dword>(owner, 260) = at<Dword>(owner, 264) = at<Dword>(owner, 268) = 0;
    at<Dword>(owner, 272) = at<Dword>(post, 4);
    at<Dword>(owner, 276) = at<Dword>(post, 8);
    const int count = int(dynamic_array_count(at<void *>(post, 0)));
    void *array = allocate_dynamic_array(16, count, nullptr);
    at<void *>(owner, 256) = array;
    if (!array)
        return 0;
    for (int i = 0; i < count; ++i) {
        void *source = dynamic_array_item(at<void *>(post, 0), i);
        alignas(4) Byte item[16];
        at<void *>(item, 0) = source;
        at<Dword>(item, 4) = Dword(-1);
        at<Dword>(item, 8) = at<Dword>(item, 12) = 0;
        bool valid = true;
        const Dword kind = at<Dword>(source, 0);
        if (kind == 1) {
            FILE *file = std::fopen(at<char *>(source, 8), "rb");
            at<FILE *>(item, 8) = file;
            valid = file && !std::fseek(file, 0, SEEK_END);
            if (valid) {
                at<int>(item, 12) = std::ftell(file);
                valid = at<int>(item, 12) != -1;
            }
            if (valid)
                std::rewind(file);
        } else if (kind != 0 && kind != 2)
            valid = false;
        if (!valid) {
            for (--i; i >= 0; --i) {
                void *old = dynamic_array_item(array, i);
                if (at<Dword>(at<void *>(old, 0), 0) == 1) {
                    if (FILE *file = at<FILE *>(old, 8))
                        std::fclose(file);
                    at<FILE *>(old, 8) = nullptr;
                }
            }
            free_dynamic_array(array);
            at<void *>(owner, 256) = nullptr;
            return 0;
        }
        append_dynamic_array(array, item);
    }
    Dword total = 0;
    if (!at<Dword>(post, 12)) {
        if (count) {
            for (int i = 0; i < count; ++i) {
                void *source = dynamic_array_item(at<void *>(post, 0), i);
                total += Dword(std::strlen(at<char *>(source, 4))) + 1 + at<Dword>(source, 20) * 2 +
                         at<Dword>(source, 12);
            }
            total += Dword(count) - 1;
        }
    } else {
        if (!multipart_boundary_bytes) {
            const Dword size = Dword(std::strlen(boundary)) + 1;
            multipart_boundary_bytes = size - 1;
            multipart_field_bytes = size + 46;
            multipart_file_bytes = size + 75;
            multipart_tail_bytes = size + 3;
        }
        for (int i = 0; i < count; ++i) {
            void *source = dynamic_array_item(at<void *>(post, 0), i);
            const Dword name = Dword(std::strlen(at<char *>(source, 4)));
            switch (at<Dword>(source, 0)) {
            case 0:
                total += name + at<Dword>(source, 12) + multipart_field_bytes;
                break;
            case 1:
                total += name + Dword(std::strlen(at<char *>(source, 12))) +
                         Dword(std::strlen(at<char *>(source, 16))) + multipart_file_bytes +
                         at<Dword>(dynamic_array_item(array, i), 12);
                break;
            case 2:
                total += name + Dword(std::strlen(at<char *>(source, 16))) +
                         Dword(std::strlen(at<char *>(source, 20))) + at<Dword>(source, 12) +
                         multipart_file_bytes;
                break;
            default:
                total = 0;
                at<Dword>(owner, 268) = total;
                return 1;
            }
        }
        total += multipart_tail_bytes;
    }
    at<Dword>(owner, 268) = total;
    return 1;
}

static bool bind_response_buffer(void *owner, Buffer *target, char *data, int capacity) {
    if (!owner || !target || !data || capacity < 1)
        return false;
    target->capacity = capacity;
    target->owner = owner;
    target->data = data;
    target->size = target->growth = 0;
    target->fixed = target->borrowed = 1;
    *data = 0;
    return true;
}

Dword submit_memory_request(const char *url, const char *headers, char *data, int capacity,
                           void *post, Dword throttle, Dword synchronous, Dword progress,
                           Dword complete, Dword context) {
    if (!url || !*url || capacity < 0 || (data && !capacity))
        return Dword(-1);
    if (!startup_count) {
        lock_requests();
        if (++startup_count == 1) {
            initialize_request_lock();
            throttle_bytes = 125;
            throttle_period = 250;
        } else
            unlock_requests();
    }
    void *owner = allocate_request();
    if (!owner)
        return Dword(-1);
    at<Dword>(owner, 8) = 0;
    at<char *>(owner, 16) = authentication::pending::duplicate(url);
    bool valid = at<char *>(owner, 16) != nullptr;
    if (valid && headers && *headers) {
        at<char *>(owner, 36) = authentication::pending::duplicate(headers);
        valid = at<char *>(owner, 36) != nullptr;
    }
    if (valid) {
        at<Dword>(owner, 44) = synchronous;
        at<Dword>(owner, 60) = context;
        at<Dword>(owner, 52) = progress;
        at<void *>(owner, 252) = post;
        at<Dword>(owner, 56) = complete;
        at<Dword>(owner, 244) = throttle;
        at<Dword>(owner, 168) = Dword(data != nullptr);
        valid = data ? bind_response_buffer(owner, buffer(owner, 136), data, capacity)
                     : initialize_buffer(owner, buffer(owner, 136), 2048, 2048);
        if (valid && post)
            valid = prepare_post_body(owner) != 0;
        if (valid) {
            if (!synchronous)
                return at<Dword>(owner, 4);
            while (!pump_request(owner))
                browser::sleep(10);
            return 0;
        }
    }
    release_request(owner);
    return Dword(-1);
}
} // namespace nfs::network::session::pending::http
