#include "../population/ai_population_runtime.hpp"
#include "../population/ai_population_x87.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {

void anchor_path_graph(void *graph, const float *position) {
    population_frame::anchor_path_graph(graph, position);
}
} // namespace nfs::game::runtime::ai::lifecycle::pending
