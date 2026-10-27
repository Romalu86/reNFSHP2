#pragma once
#include <cstdint>

namespace nfs::movies {
struct Player;
namespace mad {
struct Decoder;
}
namespace clock_native {
std::uint8_t frame_due(Player *);
void *seek(Player *, std::uint32_t frame_bits);
void initialize_frame_rate(mad::Decoder *, std::int32_t duration);
}
}
