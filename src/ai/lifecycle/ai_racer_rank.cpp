#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/retained_ai.hpp"
namespace nfs::game::runtime::ai {
// Local arrays contain eight floats and eight indices.
void rank_racers(Dword *owner) {
    const int count = at<int>(owner, 84172);
    float scores[8];
    int indices[8];
    for (int i = 0; i < count; ++i) {
        scores[i] = float(retained_priority(at<void *>(owner, 84176 + i * 4)));
        indices[i] = i;
    }
    int remaining = count;
    for (int rank = 0; rank < count; ++rank) {
        int selected = 0;
        for (int i = 1; i < remaining; ++i)
            if (scores[indices[selected]] <= scores[indices[i]])
                selected = i;
        at<int>(at<void *>(owner, 84176 + indices[selected] * 4), 576) = rank;
        indices[selected] = indices[--remaining];
    }
}
} // namespace nfs::game::runtime::ai
