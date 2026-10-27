#include "nfs/network/packets/network_bitstream.hpp"
namespace nfs::network::bits {
// Independent packet streams and their backing buffers.
Stream client_car_stream{}, client_position_stream{}, server_car_stream{};
std::uint8_t client_decoders{}, server_decoders{};
alignas(4) std::uint8_t client_car[40]{}, client_position[20]{}, server_car[40]{};
// Byte C3, followed by alignment NOPs.
void finalize_client_position() {
}
// Byte C3, followed by alignment NOPs.
void finalize_client_car() {
}
// Byte C3, followed by alignment NOPs.
void finalize_server_car() {
}
} // namespace nfs::network::bits
