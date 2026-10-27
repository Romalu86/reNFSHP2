#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/world/resources/runtime_resources.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world {


void build_path_tangents(const float *points, float *tangents, int count) {
    for (int index = 0; index < count; ++index) {
        float incoming[3], outgoing[3];
        if (index == 0) {
            for (unsigned component = 0; component < 3; ++component)
                outgoing[component] =
                    float(static_cast<long double>(points[component + 3]) - points[component]);
            runtime::normalize_vector(outgoing, tangents);
        } else if (index == count - 1) {
            for (unsigned component = 0; component < 3; ++component)
                incoming[component] =
                    float(static_cast<long double>(points[index * 3 + component]) -
                          points[index * 3 - 3 + component]);
            runtime::normalize_vector(incoming, tangents + index * 3);
        } else {
            for (unsigned component = 0; component < 3; ++component)
                incoming[component] =
                    float(static_cast<long double>(points[index * 3 + component]) -
                          points[index * 3 - 3 + component]);
            runtime::normalize_vector(incoming, incoming);
            for (unsigned component = 0; component < 3; ++component)
                outgoing[component] =
                    float(static_cast<long double>(points[index * 3 + 3 + component]) -
                          points[index * 3 + component]);
            runtime::normalize_vector(outgoing, outgoing);
            float *destination = tangents + index * 3;
            for (unsigned component = 0; component < 3; ++component)
                destination[component] =
                    float(static_cast<long double>(incoming[component]) + outgoing[component]);
            runtime::normalize_vector(destination, destination);
        }
    }
}
} // namespace nfs::game::world
