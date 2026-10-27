#pragma once
#include "nfs/physics/frame/dynamics_runtime.hpp"
namespace nfs::game::runtime::physics {

struct ContactHistory {
    float values[4][3]{};
    void append(const float *value) {
        for (unsigned i = 0; i < 3; ++i)
            copy_triplet(values[i], values[i + 1]);
        copy_triplet(values[3], value);
    }
    void average(float *output) const {
        float twice_second[3], twice_third[3], sum[3], next[3];
        scale_triplet(twice_third, values[2], 2);
        scale_triplet(twice_second, values[1], 2);
        sum_triplets(sum, values[0], twice_second);
        sum_triplets(next, sum, twice_third);
        sum_triplets(sum, next, values[3]);
        divide_triplet(output, sum, 6);
    }
};
} // namespace nfs::game::runtime::physics
