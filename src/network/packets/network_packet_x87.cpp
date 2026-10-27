#include "network_packet_x87.hpp"
#include "nfs/network/packets/network_quantizers.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>

namespace nfs::network::packet_native {
const volatile float quantizer_half = 0.5f;
const volatile float fixed_scale = 65536.0f;

// Keep raw parameter bits out of the compiler's floating-point copy lowering.
__declspec(noinline) static std::uint32_t raw_float_bits(const float *value) {
    std::uint32_t bits;
    std::memcpy(&bits, value, sizeof(bits));
    return bits;
}

static std::uint16_t read_short(const void *source) {
    std::uint16_t value;
    std::memcpy(&value, source, sizeof(value));
    return value;
}
static void write_short(void *destination, std::uint16_t value) {
    std::memcpy(destination, &value, sizeof(value));
}
static std::int32_t fixed_word(float value) {
    // Both saved vector packers spill the scaled value to F32 before FISTP DWORD.
    const float scaled = static_cast<float>(static_cast<long double>(value) * fixed_scale);
    return game::runtime::hardware::current_integer_f32(scaled);
}
std::int32_t round_fixed_word(float value) {
    return fixed_word(value);
}


bits::Quantizer *configure_quantizer(bits::Quantizer *object, const char *name,
                                    float minimum, float maximum, std::uint32_t width) {
    auto *destination = reinterpret_cast<char *>(object);
    char character;
    do {
        character = *name++;
        *destination++ = character;
    } while (character);
    const long double range = static_cast<long double>(maximum) - minimum;
    field<volatile std::uint32_t>(object, 308) = raw_float_bits(&maximum);
    field<volatile std::uint32_t>(object, 304) = raw_float_bits(&minimum);
    field<std::uint32_t>(object, 300) = width;
    const auto denominator_bits = (std::uint32_t(1) << (width & 31)) - 1;
    std::int32_t denominator;
    std::memcpy(&denominator, &denominator_bits, sizeof(denominator));
    const long double step = range / denominator;
    field<std::uint32_t>(object, 320) = 2139095039;
    field<std::uint32_t>(object, 324) = 4286578687u;
    object->bytes[328] = 1;
    field<float>(object, 316) = static_cast<float>(step);
    field<float>(object, 312) = static_cast<float>(step * quantizer_half);
    return object;
}


std::uint32_t encode_quantized(const bits::Quantizer *quantizer, float input) {
    auto *object = const_cast<bits::Quantizer *>(quantizer);
    long double value = input;
    if (value < field<float>(object, 304))
        value = field<float>(object, 304);
    if (value > field<float>(object, 308))
        value = field<float>(object, 308);
    const long double shifted = value - field<float>(object, 304);
    const long double scaled = shifted / field<float>(object, 316);
    return std::uint32_t(static_cast<std::int64_t>(scaled + quantizer_half));
}

static long double decode_value(const bits::Quantizer *quantizer, std::uint32_t value) {
    auto *object = const_cast<bits::Quantizer *>(quantizer);
    const std::int64_t unsigned_value = value;
    return static_cast<long double>(unsigned_value) * field<float>(object, 316) +
           field<float>(object, 304);
}

void decode_quantized_native80(const bits::Quantizer *quantizer, std::uint32_t value,
                               void *output) {
    const long double decoded = decode_value(quantizer, value);
    std::memcpy(output, &decoded, 10);
}


void decode_position(bits::Stream *stream, void *destination) {
    bits::read(stream, destination, 32);
    bits::Quantizer *quantizers[]{&bits::pending::position_x_quantizer,
                                 &bits::pending::position_y_quantizer,
                                 &bits::pending::position_z_quantizer};
    for (unsigned axis = 0; axis < 3; ++axis) {
        std::uint32_t value = 0;
        bits::read(stream, &value, field<int>(quantizers[axis], 300));
        field<float>(destination, 4 + axis * 4) =
            static_cast<float>(decode_value(quantizers[axis], value));
    }
}


std::uint8_t snapshot_due(float elapsed, float previous) {
    const long double delta = static_cast<long double>(elapsed) - previous;
    return delta >= 0.125f || elapsed < previous;
}
std::uint32_t clock_ticks(float elapsed, float scale) {
    return std::uint32_t(static_cast<std::int64_t>(static_cast<long double>(elapsed) * scale));
}

// External linkage and noinline preserve the Win32 float return in ST0.
// Keep this provider out of an anonymous namespace: a local-only calling
// convention can turn a float transfer back into an integer copy.
__declspec(noinline) float load_packet_speed(const volatile float *source) {
    return *source;
}

void apply_car_speeds(void *car, const void *speed_fields) {
    const auto *speeds = static_cast<const volatile float *>(speed_fields);
    // Complete the FLD/FSTP of the average before loading the current speed.
    field<volatile float>(car, 1856) = load_packet_speed(speeds + 1);
    field<volatile float>(car, 1852) = load_packet_speed(speeds);
}


void pack_position(const float *position, void *output) {
    auto *packed = static_cast<std::uint8_t *>(output);
    float value = position[0];
    for (unsigned axis = 0; axis < 3; ++axis) {
        if (value < 0)
            value = -value;
        const std::int32_t fixed = fixed_word(value);
        const bool negative = position[axis] < 0;
        const auto magnitude = std::uint16_t((read_short(packed + axis * 2) & 32768) |
                                              (std::uint32_t(fixed >> 16) & 32767));
        write_short(packed + axis * 2, magnitude);
        if (axis < 2)
            value = position[axis + 1];
        const auto signed_magnitude =
            std::uint16_t((magnitude & 32767) | (negative ? 32768 : 0));
        if (axis == 0) {
            packed[6] = std::uint8_t(std::uint32_t(fixed) >> 8);
            write_short(packed, signed_magnitude);
        } else {
            write_short(packed + axis * 2, signed_magnitude);
            packed[6 + axis] = std::uint8_t(std::uint32_t(fixed) >> 8);
        }
    }
}


void pack_velocity(const float *velocity, void *output) {
    auto *packed = static_cast<std::uint8_t *>(output);
    float value = velocity[0];
    for (unsigned axis = 0; axis < 3; ++axis) {
        if (value < 0)
            value = -value;
        const std::int32_t fixed = fixed_word(value);
        // X and Y update the same WORD; Z updates the following WORD.
        const unsigned word_offset = axis == 1 ? 0 : axis;
        const unsigned bit_offset = axis == 1 ? 8 : 0;
        const std::uint32_t magnitude_mask = 127u << bit_offset;
        const std::uint32_t sign_mask = 128u << bit_offset;
        if ((fixed >> 16) <= 127) {
            write_short(packed + word_offset,
                        std::uint16_t((read_short(packed + word_offset) & ~magnitude_mask) |
                                      ((std::uint32_t(fixed >> 16) & 127) << bit_offset)));
            packed[axis + 3] = std::uint8_t(std::uint32_t(fixed) >> 8);
        } else {
            packed[axis] |= 127;
            packed[axis + 3] |= 255;
        }
        const bool negative = velocity[axis] < 0;
        if (axis < 2)
            value = velocity[axis + 1];
        write_short(packed + word_offset,
                    std::uint16_t((read_short(packed + word_offset) & ~sign_mask) |
                                  (negative ? sign_mask : 0)));
    }
}
} // namespace nfs::network::packet_native
