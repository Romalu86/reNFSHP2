#include "nfs/world/resources/runtime_resources.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cmath>
namespace nfs::game::runtime {

void reset_car_particle_pool(Dword *object) {
    at<Dword>(object, 72000) = 0;
    for (unsigned index = 0; index < 1000; ++index)
        at<void *>(object, 68000 + index * 4) = bytes(object) + index * 68;
}

void update_car_particle_pool(Dword *object) {
    using kernel::numeric::Extended;
    const auto format = kernel::numeric::current_format();
    auto E = [&](float value) { return Extended(value, format); };
    int index = 0, count = at<int>(object, 72000);
    while (index < count) {
        auto *particle = at<float *>(object, 68000 + index * 4);
        const auto age = E(0.0333333350718021392822265625f) + particle[3];
        particle[3] = float(age);
        const auto relative_age = age * particle[7];
        if (relative_age < E(1) || std::isnan(double(relative_age))) {
            void *settings = at<void *>(particle, 60);
            float delta[3];
            scale_vectors(1, particle + 4, 0.0333333350718021392822265625f, delta);
            for (unsigned axis = 0; axis < 3; ++axis)
                particle[axis] = float(E(delta[axis]) + particle[axis]);
            scale_vectors(1, particle + 4, at<float>(settings, 212), particle + 4);
            for (unsigned axis = 0; axis < 3; ++axis)
                particle[axis + 4] = float(E(particle[axis + 4]) +
                                           at<float>(settings, 328 + axis * 4));
            particle[12] =
                float(E(0.0333333350718021392822265625f) * particle[13] + particle[12]);
            ++index;
        } else {
            --count;
            at<int>(object, 72000) = count;
            if (index != count) {
                at<float *>(object, 68000 + index * 4) = at<float *>(object, 68000 + count * 4);
                at<float *>(object, 68000 + at<int>(object, 72000) * 4) = particle;
            }
        }
        count = at<int>(object, 72000);
    }
}
} // namespace nfs::game::runtime
