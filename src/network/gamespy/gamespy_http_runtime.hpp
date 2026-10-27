#pragma once
#include "gamespy_query_runtime.hpp"
#include "gamespy_http_strings.hpp"
#include <cctype>
namespace nfs::network::session::pending::http {
struct Buffer {
    void *owner;
    char *data;
    int capacity, size, position, growth;
    Dword fixed, borrowed;
};
static_assert(sizeof(Buffer) == 32);
extern CRITICAL_SECTION *request_lock;
extern int active_requests, request_capacity;
extern void **requests;
extern char *proxy_host;
extern std::uint16_t proxy_port;
extern int throttle_bytes;
extern Dword throttle_period;
void initialize_request_lock();
void lock_requests();
void unlock_requests();
void *allocate_request();
Dword release_request(void *);
void *find_request(int);
void notify_complete(void *);
void notify_progress(void *, Dword, Dword);
void notify_upload(void *);
void redirect_request(void *);
int pump_request(void *);
void visit_requests(int(__cdecl *)(void *));
bool initialize_buffer(void *, Buffer *, int, int);
void destroy_buffer(Buffer *);
bool append_buffer(Buffer *, const void *, int);
bool append_header(Buffer *, const char *, const char *);
bool append_character(Buffer *, char);
void append_integer(Buffer *, Dword);
void clear_buffer(Buffer *);
bool flush_request(Buffer *, void *);
bool poll_socket(SOCKET, int *, int *, int *);
int receive_request(void *, char *, int *);
int send_request(void *, const char *, int);
Byte send_or_queue(void *, const char *, int);
bool set_blocking(SOCKET, Dword);
bool set_receive_buffer(SOCKET, int);
void resolve_request(void *);
void connect_request(void *);
void send_headers(void *);
void send_post_body(void *);
void wait_for_response(void *);
void read_status(void *);
void read_headers(void *);
void read_body(void *);
Dword consume_body(void *, const char *, int);
Dword consume_chunked(void *, const char *, int);
void destroy_post_body(void *);
Byte pump_post_body(void *);
void free_dynamic_array(void *);
void *allocate_dynamic_array(int, int, void(__cdecl *)(void *));
void append_dynamic_array(void *, const void *);
Dword dynamic_array_count(void *);
void *dynamic_array_item(void *, int);
const char *post_content_type(void *);
inline Buffer *buffer(void *owner, unsigned offset) {
    return &at<Buffer>(owner, offset);
}
inline char *text(void *owner, unsigned offset) {
    return reinterpret_cast<char *>(bytes(owner) + offset);
}
inline void fail(void *owner, Dword result) {
    at<Dword>(owner, 188) = 1;
    at<Dword>(owner, 48) = result;
}
} // namespace nfs::network::session::pending::http
