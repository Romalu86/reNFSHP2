#pragma once
#include <cstdint>
namespace nfs::network::bits {
struct Quantizer;
struct Stream;
}
namespace nfs::network::packet_native {
bits::Quantizer *configure_quantizer(bits::Quantizer *, const char *, float minimum,
                                    float maximum, std::uint32_t width);
std::uint32_t encode_quantized(const bits::Quantizer *, float value);
void decode_quantized_native80(const bits::Quantizer *, std::uint32_t value, void *output);
void decode_position(bits::Stream *, void *output);
std::uint8_t snapshot_due(float elapsed, float previous);
std::uint32_t clock_ticks(float elapsed, float scale);
std::int32_t round_fixed_word(float value);
void pack_position(const float *, void *);
void pack_velocity(const float *, void *);
void apply_car_speeds(void *car, const void *speed_fields);
}
