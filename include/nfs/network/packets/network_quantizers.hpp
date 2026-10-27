#pragma once
#include "nfs/network/packets/network_bitstream.hpp"
namespace nfs::network::bits {
Quantizer *construct_quantizer(Quantizer *);
void destroy_quantizer(Quantizer *);
Quantizer *configure_bits(Quantizer *, const char *, float, float, Dword);
void configure_quantizers();
} // namespace nfs::network::bits
