#include "race_particle_render.hpp"
#include "nfs/graphics/particles/particle_lifecycle.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::runtime::particles::lifecycle {
using kernel::numeric::Extended;

void append_render_particle(Renderer *renderer, Particle *particle) {
    const auto format = kernel::numeric::current_format();
    const auto factor = Extended(1.f, format) / Extended(50.f, format);
    int index = int(Dword((factor * particle->depth).truncate_to_integer()));
    if (index >= 18)
        index = 17;
    else if (index < 0)
        index = 0;
    auto &bucket = renderer->buckets[index];
    const int slot = renderer->count;
    if (bucket.head == -1)
        bucket.head = slot;
    else
        renderer->next[bucket.tail] = slot;
    bucket.tail = slot;
    ++renderer->count;
    renderer->particles[bucket.tail] = particle;
    renderer->next[bucket.tail] = -1;
}

void reset_render_particles(Renderer *renderer) {
    renderer->count = 0;
    for (auto &bucket : renderer->buckets)
        bucket.head = bucket.tail = -1;
}

void prepare_particle_groups(Renderer *renderer, void *camera) {
    Dword *pose = invoke<Dword *>(camera, 2);
    float orientation[9];
    race_audio::camera_orientation(pose, orientation);
    renderer->right[2] = -orientation[2];
    renderer->right[0] = orientation[0];
    renderer->up[2] = -orientation[5];
    renderer->up[1] = orientation[4];
    renderer->right[1] = orientation[1];
    renderer->up[0] = orientation[3];
    int count = 0;
    for (auto &bucket : renderer->buckets) {
        int index = bucket.head;
        bucket.first = count;
        while (index != -1) {
            renderer->sorted[count++] = renderer->particles[index];
            index = renderer->next[index];
        }
        bucket.last = count;
    }
    const auto format = kernel::numeric::current_format();
    for (auto &bucket : renderer->buckets)
        if (bucket.first != bucket.last) {
            auto *owner = at<Renderer *>(nfs::lifecycle::runtime::particle_owner, 8);
            bucket.depth = float((Extended(owner->sorted[bucket.last - 1]->depth, format) +
                                  owner->sorted[bucket.first]->depth) * 0.5f);
            graphics::deferred::enqueue(&bucket.drawable);
        }
}

void queue_particle_groups(void *owner, void *camera) {
    prepare_particle_groups(at<Renderer *>(owner, 8), camera);
}

void reset_particle_groups(void *owner) {
    reset_render_particles(at<Renderer *>(owner, 8));
}
} // namespace nfs::game::runtime::particles::lifecycle
