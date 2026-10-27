#include "gamespy_http_runtime.hpp"
namespace nfs::network::session::pending::http {
CRITICAL_SECTION *request_lock{};
int active_requests{}, request_capacity{};
void **requests{};
char *proxy_host{};
std::uint16_t proxy_port{};
int throttle_bytes = 125;
Dword throttle_period = 250;

void initialize_request_lock() {
    auto *lock = static_cast<CRITICAL_SECTION *>(std::malloc(24));
    if (lock)
        InitializeCriticalSection(lock);
    request_lock = lock;
}

void lock_requests() {
    if (request_lock)
        EnterCriticalSection(request_lock);
}

void unlock_requests() {
    if (request_lock)
        LeaveCriticalSection(request_lock);
}

void notify_complete(void *owner) {
    if (auto callback = at<int(__cdecl *)(Dword, Dword, void *, Dword, Dword)>(owner, 56)) {
        void *data = at<Dword>(owner, 8) == 0 ? at<void *>(owner, 140) : nullptr;
        const Dword size = at<Dword>(owner, 8) == 0 ? at<Dword>(owner, 192) : 0;
        const int retained =
            callback(at<Dword>(owner, 4), at<Dword>(owner, 48), data, size, at<Dword>(owner, 60));
        if (data && !retained)
            at<Dword>(owner, 164) = 1;
    }
}

void notify_progress(void *owner, Dword first, Dword second) {
    if (auto callback = at<void(__cdecl *)(Dword, Dword, Dword, Dword, Dword, Dword, Dword)>(owner, 52))
        callback(at<Dword>(owner, 4), at<Dword>(owner, 12), first, second, at<Dword>(owner, 192),
                 at<Dword>(owner, 196), at<Dword>(owner, 60));
}

Dword dynamic_array_count(void *array) {
    return at<Dword>(array, 0);
}

void *dynamic_array_item(void *array, int index) {
    return bytes(at<void *>(array, 20)) + at<Dword>(array, 8) * Dword(index);
}

void free_dynamic_array(void *array) {
    if (at<int>(array, 0) > 0) {
        const auto destroy = at<void(__cdecl *)(void *)>(array, 16);
        for (int i = 0; i < at<int>(array, 0); ++i)
            if (destroy)
                destroy(dynamic_array_item(array, i));
    }
    std::free(at<void *>(array, 20));
    std::free(array);
}

void notify_upload(void *owner) {
    if (auto callback = at<void(__cdecl *)(Dword, Dword, Dword, Dword, Dword, Dword)>(owner, 272)) {
        const Dword context = at<Dword>(owner, 60);
        const Dword count = dynamic_array_count(at<void *>(owner, 256));
        callback(at<Dword>(owner, 4), at<Dword>(owner, 264), at<Dword>(owner, 268),
                 at<Dword>(owner, 260), count, context);
    }
}

void destroy_post_body(void *owner) {
    if (void *array = at<void *>(owner, 256)) {
        const int count = int(dynamic_array_count(array));
        for (int i = 0; i < count; ++i) {
            void *item = dynamic_array_item(array, i);
            void *source = at<void *>(item, 0);
            if (at<Dword>(source, 0) == 1) {
                if (FILE *file = at<FILE *>(item, 8))
                    std::fclose(file);
                at<FILE *>(item, 8) = nullptr;
            }
        }
        free_dynamic_array(array);
        at<void *>(owner, 256) = nullptr;
    }
    if (void *post = at<void *>(owner, 252))
        if (at<Dword>(post, 16)) {
            free_dynamic_array(at<void *>(post, 0));
            std::free(post);
            at<void *>(owner, 252) = nullptr;
        }
}

static Dword post_owned(void *post) {
    return at<Dword>(post, 16);
}

static void destroy_post(void *post) {
    free_dynamic_array(at<void *>(post, 0));
    std::free(post);
}

Dword release_request(void *owner) {
    if (!owner || !at<Dword>(owner, 0) || at<int>(owner, 4) < 0 ||
        at<int>(owner, 4) >= request_capacity)
        return 0;
    lock_requests();
    for (unsigned offset : {16u, 20u, 32u, 36u, 200u})
        std::free(at<void *>(owner, offset));
    if (FILE *file = at<FILE *>(owner, 40))
        std::fclose(file);
    const SOCKET handle = at<SOCKET>(owner, 64);
    if (handle != INVALID_SOCKET) {
        shutdown(handle, 2);
        closesocket(handle);
    }
    destroy_buffer(buffer(owner, 72));
    destroy_buffer(buffer(owner, 104));
    destroy_buffer(buffer(owner, 136));
    if (at<void *>(owner, 256))
        destroy_post_body(owner);
    if (void *post = at<void *>(owner, 252))
        if (post_owned(post)) {
            destroy_post(post);
            at<void *>(owner, 252) = nullptr;
        }
    --active_requests;
    at<Dword>(owner, 0) = 0;
    unlock_requests();
    return 1;
}

void *allocate_request() {
    lock_requests();
    int index = 0;
    for (; index < request_capacity; ++index)
        if (!at<Dword>(requests[index], 0))
            break;
    if (index == request_capacity) {
        const int capacity = int(Dword(request_capacity) + 4);
        auto **resized = static_cast<void **>(std::realloc(requests, Dword(capacity) * 4));
        if (!resized) {
            unlock_requests();
            return nullptr;
        }
        requests = resized;
        for (int i = index; i < capacity; ++i) {
            requests[i] = std::malloc(280);
            if (!requests[i]) {
                for (--i; i >= index; --i)
                    std::free(requests[i]);
                unlock_requests();
                return nullptr;
            }
            at<Dword>(requests[i], 0) = 0;
        }
        request_capacity = capacity;
    }
    void *owner = requests[index];
    std::memset(owner, 0, 280);
    at<Dword>(owner, 0) = 1;
    at<int>(owner, 4) = index;
    at<SOCKET>(owner, 64) = INVALID_SOCKET;
    at<Dword>(owner, 196) = Dword(-1);
    if (initialize_buffer(owner, buffer(owner, 72), 2048, 4096) &&
        initialize_buffer(owner, buffer(owner, 104), 2048, 2048)) {
        ++active_requests;
        unlock_requests();
        return owner;
    }
    release_request(owner);
    unlock_requests();
    return nullptr;
}

void *find_request(int index) {
    lock_requests();
    void *owner = index >= 0 && index < request_capacity ? requests[index] : nullptr;
    if (owner && !at<Dword>(owner, 0))
        owner = nullptr;
    unlock_requests();
    return owner;
}

void visit_requests(int(__cdecl *visitor)(void *)) {
    if (active_requests > 0) {
        lock_requests();
        for (int i = 0; i < request_capacity; ++i) {
            void *owner = requests[i];
            if (at<Dword>(owner, 0))
                visitor(owner);
        }
        unlock_requests();
    }
}

void redirect_request(void *owner) {
    at<Dword>(owner, 12) = 0;
    std::free(at<void *>(owner, 16));
    at<void *>(owner, 16) = at<void *>(owner, 200);
    at<void *>(owner, 200) = nullptr;
    std::free(at<void *>(owner, 20));
    at<void *>(owner, 20) = nullptr;
    at<Dword>(owner, 24) = 0;
    at<std::uint16_t>(owner, 28) = 0;
    std::free(at<void *>(owner, 32));
    at<void *>(owner, 32) = nullptr;
    shutdown(at<SOCKET>(owner, 64), 2);
    closesocket(at<SOCKET>(owner, 64));
    at<SOCKET>(owner, 64) = INVALID_SOCKET;
    clear_buffer(buffer(owner, 72));
    clear_buffer(buffer(owner, 104));
    for (unsigned offset : {172u, 176u, 180u, 184u, 240u})
        at<Dword>(owner, offset) = 0;
    ++at<Dword>(owner, 204);
}

int pump_request(void *owner) {
    if (at<Dword>(owner, 236))
        return 0;
    at<Dword>(owner, 236) = 1;
    if (at<Dword>(owner, 12) == 0)
        resolve_request(owner);
    if (at<Dword>(owner, 12) == 1)
        connect_request(owner);
    if (at<Dword>(owner, 12) == 2)
        send_headers(owner);
    if (at<Dword>(owner, 12) == 3)
        send_post_body(owner);
    if (at<Dword>(owner, 12) == 4)
        wait_for_response(owner);
    if (at<Dword>(owner, 12) == 5)
        read_status(owner);
    if (at<Dword>(owner, 12) == 6)
        read_headers(owner);
    if (at<Dword>(owner, 12) == 7)
        read_body(owner);
    if (at<void *>(owner, 200))
        redirect_request(owner);
    const int complete = at<int>(owner, 188);
    if (complete) {
        const int status = at<int>(owner, 180);
        switch (status / 100) {
        case 4:
            switch (status) {
            case 401:
                at<Dword>(owner, 48) = 9;
                break;
            case 403:
                at<Dword>(owner, 48) = 10;
                break;
            case 404:
            case 410:
                at<Dword>(owner, 48) = 11;
                break;
            default:
                at<Dword>(owner, 48) = 8;
                break;
            }
            break;
        case 5:
            at<Dword>(owner, 48) = 12;
            break;
        }
        if (FILE *file = at<FILE *>(owner, 40)) {
            std::fclose(file);
            at<FILE *>(owner, 40) = nullptr;
        }
        notify_complete(owner);
        release_request(owner);
        return complete;
    }
    at<Dword>(owner, 236) = 0;
    return 0;
}
} // namespace nfs::network::session::pending::http
namespace nfs::network::session::pending {

void browser_think() {
    http::visit_requests(http::pump_request);
}
} // namespace nfs::network::session::pending
