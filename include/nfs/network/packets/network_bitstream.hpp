#pragma once
#include "nfs/network/session/network_runtime.hpp"
namespace nfs::network::bits {
struct Stream {
    std::uint8_t data[536];
    Dword written, read;
    std::uint8_t writing;
};
static_assert(offsetof(Stream, written) == 536 && sizeof(Stream) == 548);
struct Quantizer {
    alignas(4) std::uint8_t bytes[336];
};
namespace pending {
extern Quantizer position_x_quantizer, position_y_quantizer, position_z_quantizer;
}
Stream *construct(Stream *);
void reset(Stream *);
void bind(Stream *, const void *, Dword);
void write(Stream *, Dword, int);
void read(Stream *, void *, int);
long double decode_quantized(const Quantizer *, Dword);
Dword encode_quantized(const Quantizer *, float);
void encode_car(Stream *, void *);
void encode_position(Stream *, void *);
void encode_server_car(Stream *, void *);
void encode_server_position(Stream *, void *);
void decode_car(Stream *, void *);
void decode_position(Stream *, void *);
void decode_client_car(Stream *, void *);
void decode_server_car(Stream *, void *);
void decode_server_position(Stream *, void *);
extern Stream client_car_stream, client_position_stream, server_car_stream;
extern std::uint8_t client_decoders, server_decoders;
alignas(4) extern std::uint8_t client_car[40], client_position[20], server_car[40];
void finalize_client_car();
void finalize_client_position();
void finalize_server_car();
} // namespace nfs::network::bits
