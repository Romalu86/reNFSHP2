#include "nfs/audio/backend/sound_backend.hpp"
namespace nfs::sound::backend {
extern const std::uint16_t sine_quadrant[257];
extern const float pan_vector_scale;

int pending::sine_table(int angle) {
    unsigned index = Dword(angle) & 255, quadrant = (Dword(angle) >> 8) & 3;
    int value = sine_quadrant[(quadrant & 1) ? 256 - index : index];
    return quadrant & 2 ? -value : value;
}

void voice_pan_vector(Dword horizontal, Dword vertical, float *output) {
    int elevation = -int((int(vertical) >> 6) & 1023);
    int y = pending::sine_table(elevation + 1024),
        radius = pending::sine_table(elevation + 1280),
        x = pending::sine_table(int(horizontal) >> 6),
        z = pending::sine_table((int(horizontal) >> 6) + 256);
    output[0] = float(static_cast<long double>(float(x)) * float(radius) * pan_vector_scale);
    output[1] = float(static_cast<long double>(y) * -1.0f);
    output[2] = float(static_cast<long double>(float(z)) * float(radius) * pan_vector_scale);
}
} // namespace nfs::sound::backend
