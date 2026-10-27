#include "nfs/audio/backend/sound_ownership.hpp"
#include "../../math/vectors/frame_x87_private.hpp"
namespace nfs::sound::backend::pending {

void mix_scalar(int count, float gain, const float *input, float *output) {

    auto combine = [&](float a, float b) {
        return float(static_cast<long double>(a) * gain + b);
    };
    Dword n = Dword(count);
    while (n & 3) {
        output[n - 1] = combine(input[n - 1], output[n - 1]);
        if (!--n)
            return;
    }
    do {
        Dword p = n - 4;
        float b = input[p + 1], c = input[p + 2], d = input[p + 3];
        output[p] = combine(input[p], output[p]);
        output[p + 1] = combine(b, output[p + 1]);
        output[p + 3] = combine(d, output[p + 3]);
        output[p + 2] = combine(c, output[p + 2]);
        Dword before = n;
        n -= 4;
        if (!n || int(before) <= 3)
            break;
    } while (true);
}
}
