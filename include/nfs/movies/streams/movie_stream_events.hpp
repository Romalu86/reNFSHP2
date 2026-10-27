#pragma once
#include <cstdint>

namespace nfs::files::streaming {
struct Pool;
}
namespace nfs::movies {
struct Player;
// Movie-owned failure propagation. No failure policy is installed for other streams.
void bind_stream_owner(Player *);
void unbind_stream_owner(Player *) noexcept;
bool stream_failed(Player *) noexcept;
bool notify_stream_open_failed(files::streaming::Pool *, std::uint32_t operation,
                               int status) noexcept;
}
