#include "movie_clock_x87.hpp"
#include "movie_mad.hpp"
#include "../../math/precision/frame_x87_bridge.hpp"
#include <cstring>

namespace nfs::movies::clock_native {
namespace {
// The FMUL/FADD operands are DWORDs, not double constants.
const volatile float milliseconds_per_second = 0.001f;
const volatile float one_frame = 1.0f;
const volatile float mad_duration_scale = 0.00001525902189314365386962890625f;
const volatile float milliseconds_in_second = 1000.0f;

__declspec(noinline) long double load_frame_for_conversion(Dword bits) {
    float frame;
    std::memcpy(&frame, &bits, sizeof frame);
    return static_cast<long double>(frame);
}

long double decoder_frame_rate(Delegate *delegate) {
    if (!delegate->decoder)
        return 0.0L;

    // MSVC provider also returns this F32 field in ST0, with no class result.
    using FrameRate = float(__thiscall *)(void *);
    auto *decoder = delegate->decoder;
    auto *methods = *static_cast<const Dword **>(decoder);
    return reinterpret_cast<FrameRate>(methods[4])(decoder);
}
}

void initialize_frame_rate(mad::Decoder *decoder, std::int32_t duration) {
    const long double milliseconds = static_cast<long double>(duration) * mad_duration_scale;
    decoder->rate = static_cast<float>(milliseconds_in_second / milliseconds);
}


std::uint8_t frame_due(Player *player) {
    const int elapsed_time = elapsed(player);
    const long double rate = decoder_frame_rate(player->delegate);
    const auto milliseconds = std::int32_t(Dword(player->offset) + Dword(elapsed_time));
    const Dword displayed_frame = static_cast<const volatile Dword &>(player->frame);
    const long double scaled_rate = rate * milliseconds_per_second;
    const long double target = scaled_rate * milliseconds;
    player->target_frame = static_cast<float>(target);
    const std::int64_t displayed = displayed_frame;
    const long double next_frame = static_cast<long double>(displayed) + one_frame;
    return !(target < next_frame);
}


// truncating FISTP QWORD under a saved/restored control word, like __ftol.
void *seek(Player *player, Dword frame_bits) {
    const long double frame = load_frame_for_conversion(frame_bits);
    // The displayed argument is a raw DWORD even when its floating load changes
    // a signaling NaN for the subsequent integer conversion.
    *reinterpret_cast<volatile Dword *>(&player->target_frame) = frame_bits;
    const auto integral = game::runtime::hardware::truncate_integer_native80(&frame);
    const int requested = int(Dword(integral));
    void *result = decode_frame(player->delegate, requested);
    player->frame = current_frame(player->delegate);
    return result;
}
} // namespace nfs::movies::clock_native
