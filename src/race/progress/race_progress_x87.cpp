#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../rules/race_rules_x87.hpp"

namespace nfs::game::runtime::ai::race_progress_x87 {
// The public MSVC class result crosses this boundary as ten bytes.
void priority_native80(void *vehicle, void *output) {
    const float progress = at<float>(vehicle, 564);
    long double value;
    if (progress == -1000000.0f)
        value = -1000000.0f;
    else {
        Dword *owner = system(76);
        const Dword index = invoke<Dword>(vehicle, 15);
        const float section_length = at<float>(owner, index * 42080 + 16868);
        const auto lap = static_cast<long double>(at<int>(vehicle, 568)) + 1.0f;
        value = static_cast<long double>(section_length) * lap - at<float>(vehicle, 564);
    }
    std::memcpy(output, &value, 10);
}
}
