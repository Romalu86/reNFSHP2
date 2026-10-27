#pragma once
#include "race_deferred_draw.hpp"
namespace nfs::game::runtime::particles::lifecycle {
using kernel::Dword;
struct Particle {
    float position[3], age, velocity[3], inverse_lifetime;
    float width, height, width_delta, height_delta, angle, angular_velocity, depth;
    void *definition;
    Dword colour;
};
struct RenderBucket {
    float depth;
    int head, tail, first, last;
    graphics::deferred::Drawable drawable;
};
struct Renderer {
    int count;
    Particle *particles[1000];
    int next[1000];
    Particle *sorted[1000];
    RenderBucket buckets[18];
    Dword untextured_state[17], textured_state[17];
    float right[3], up[3];
};
static_assert(sizeof(Particle) == 68 && sizeof(RenderBucket) == 28 &&
              offsetof(Renderer, sorted) == 8004 && offsetof(Renderer, buckets) == 12004 &&
              sizeof(Renderer) == 12668);
void append_render_particle(Renderer *, Particle *);
void prepare_particle_groups(Renderer *, void *);
void reset_render_particles(Renderer *);
void queue_particle_groups(void *, void *);
void reset_particle_groups(void *);
void collect_visible_particles(void *, void *);
void collect_particle_groups(void *, void *);
}
