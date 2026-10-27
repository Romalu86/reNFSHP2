#include "frontend_particle_widget.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/physics/frame/simulation_runtime.hpp"
#include "../../gui/lifecycle/gui_render_numeric_x87.hpp"
#include <cmath>
#include <cstring>

#pragma STDC FENV_ACCESS ON
#include <cstdlib>
namespace nfs::application::frontend {
using gui::retained::number;
using gui::retained::integer;

namespace {
void copy_float_bits(float &destination, const float &source) {
    std::memcpy(&destination, &source, sizeof(float));
}
}

float menu_particle_period(float spawn_rate) {
    return float(1.0L / spawn_rate);
}

MenuParticle *menu_particle_at(ParticleWidget *widget, int index) {
    return widget->particles && index >= 0 && index < widget->maximum_particles
               ? widget->particles + index
               : nullptr;
}
// Equal keys deliberately return-1 in the saved comparator.
static int __cdecl compare_particle_order(const void *left, const void *right) {
    return static_cast<const MenuParticle *>(left)->order >
                   static_cast<const MenuParticle *>(right)->order
               ? 1
               : -1;
}

void spawn_menu_particle(ParticleWidget *widget) {
    if (!widget->particles || widget->maximum_particles <= 0)
        return;
    MenuParticle *selected = widget->particles;
    for (int index = 0; index < widget->maximum_particles; ++index) {
        MenuParticle *particle = menu_particle_at(widget, index);
        if (!particle)
            return;
        if (particle->remaining == 0) {
            selected = particle;
            break;
        }
        if (particle->remaining < selected->remaining)
            selected = particle;
    }
    // This first length is discarded by the original, but its FPU effects remain.
    const auto initial_y = number(widget->velocity[1]);
    const auto initial_x = number(widget->velocity[0]);
    const float initial_squared = float(initial_x * initial_x + initial_y * initial_y);
    volatile float initial_length = float(__builtin_sqrtl(number(initial_squared)));
    (void)initial_length;
    kernel::memory_zero(selected, sizeof(MenuParticle));
    copy_float_bits(selected->remaining, widget->life);
    auto E = [](float value) { return number(value); };
    auto I = [](int value) { return integer(value); };
    auto random_fraction = [&] { return I(int(network::rules::random() & 65535)) * 0.0000152587890625f; };
    const auto x = random_fraction();
    const int half_width = widget->base.base.bounds.width / 2;
    selected->x = float((x - .5f) * E(widget->scatter_x) + I(half_width));
    const auto y = random_fraction();
    const int half_height = widget->base.base.bounds.height / 2;
    copy_float_bits(selected->velocity_x, widget->velocity[0]);
    copy_float_bits(selected->velocity_y, widget->velocity[1]);
    selected->y = float((y - .5f) * E(widget->scatter_y) + I(half_height));
    if (widget->spread != 0) {
        const float y_velocity = selected->velocity_y, x_velocity = selected->velocity_x;
        const float squared = float(E(x_velocity) * E(x_velocity) + E(y_velocity) * E(y_velocity));
        const float length = float(__builtin_sqrtl(E(squared)));
        const float direction = float(
            game::runtime::turn_direction_retained(x_velocity, y_velocity) * 360.f);
        const float turns =
            float(((random_fraction() - .5f) * E(widget->spread) + E(direction)) * 0.00277777784503996372222900390625f);
        selected->velocity_x =
            float(game::runtime::turn_sine(turns) * length);
        selected->velocity_y = float(
            game::runtime::turn_cosine(turns) * length);
    }
    selected->red = float(I(int((widget->start_colour >> 16) & 255)) * 0.0039215688593685626983642578125f);
    selected->green = float(I(int((widget->start_colour >> 8) & 255)) * 0.0039215688593685626983642578125f);
    selected->blue = float(I(int(widget->start_colour & 255)) * 0.0039215688593685626983642578125f);
    selected->alpha = float(I(int(widget->start_colour >> 24)) * 0.0039215688593685626983642578125f);
    const auto angle_random = random_fraction();
    copy_float_bits(selected->scale, widget->scale);
    selected->angle =
        float((angle_random * E(widget->angle_randomness) + E(widget->angle)) * 0.00277777784503996372222900390625f);
    const auto spin_random = random_fraction();
    const int sort_direction = widget->sort_direction;
    std::memcpy(&selected->order, &widget->serial, sizeof(Dword));
    widget->serial += Dword(sort_direction);
    selected->angular_velocity =
        float(((spin_random - .5f) * E(widget->angular_spread) + E(widget->angular_velocity)) *
              0.00277777784503996372222900390625f);
    if (sort_direction)
        std::qsort(widget->particles, std::size_t(widget->maximum_particles), sizeof(MenuParticle),
                   compare_particle_order);
}

void advance_menu_particles(ParticleWidget *widget, float seconds) {
    if (widget->life == 0)
        return;
    auto E = [](float value) { return number(value); };
    auto normalized_colour = [&](Dword colour, float *values) {
        const unsigned shifts[] = {16, 8, 0, 24};
        for (unsigned index = 0; index != 4; ++index)
            values[index] = float(integer(int((colour >> shifts[index]) & 255)) *
                                  0.0039215688593685626983642578125f);
    };
    float start[4], middle[4], end[4];
    normalized_colour(widget->start_colour, start);
    normalized_colour(widget->middle_colour, middle);
    normalized_colour(widget->end_colour, end);
    const auto delta = E(seconds);
    MenuParticle *particle = widget->particles;
    for (int index = 0; index < widget->maximum_particles; ++index, ++particle) {
        if (!widget->particles || index < 0 || index >= widget->maximum_particles || !particle)
            return;
        if (particle->remaining == 0)
            continue;
        const auto retained_progress = E(1) - E(particle->remaining) / E(widget->life);
        const float progress = float(retained_progress);
        auto first_weight = retained_progress, second_weight = E(1) - retained_progress;
        const float *first_colour = start, *second_colour = end;
        bool reverse_product_order = false;
        if (widget->middle_life != 0) {
            if (retained_progress < E(widget->middle_life)) {
                const auto base = E(1) - E(progress) / E(widget->middle_life);
                first_weight =
                    game::runtime::physics::legacy::power_retained(double(base), double(widget->fade_in));
                second_weight = E(1) - first_weight;
                second_colour = middle;
            } else {
                const auto base =
                    (E(progress) - E(widget->middle_life)) / (E(1) - E(widget->middle_life));
                first_weight =
                    game::runtime::physics::legacy::power_retained(double(base), double(widget->fade_out));
                second_weight = E(1) - first_weight;
                first_colour = end;
                second_colour = middle;
                reverse_product_order = true;
            }
        }
        float *channels[] = {&particle->red, &particle->green, &particle->blue, &particle->alpha};
        for (unsigned channel = 0; channel != 4; ++channel) {
            if (reverse_product_order) {
                const auto first_product = first_weight * E(first_colour[channel]);
                const auto second_product = second_weight * E(second_colour[channel]);
                *channels[channel] = float(first_product + second_product);
            } else {
                const auto second_product = E(second_colour[channel]) * second_weight;
                const auto first_product = first_weight * E(first_colour[channel]);
                *channels[channel] = float(second_product + first_product);
            }
        }
        const auto velocity_x = delta * E(widget->acceleration[0]) + E(particle->velocity_x);
        const auto velocity_y = delta * E(widget->acceleration[1]) + E(particle->velocity_y);
        const float damped_x = float(velocity_x - velocity_x * E(widget->drag[0]) * delta);
        particle->velocity_x = damped_x;
        const auto damped_y = velocity_y - velocity_y * E(widget->drag[1]) * delta;
        particle->velocity_y = float(damped_y);
        particle->x = float(E(damped_x) * delta + E(particle->x));
        particle->y = float(damped_y * delta + E(particle->y));
        const auto angular_velocity =
            E(particle->angular_velocity) -
            E(widget->angular_drag) * E(particle->angular_velocity) * delta;
        particle->angular_velocity = float(angular_velocity);
        particle->angle = float(angular_velocity * delta + E(particle->angle));
        particle->scale = float(delta * E(widget->growth) + E(particle->scale));
        const auto remaining = E(particle->remaining) - delta;
        const bool alive = remaining > 0.0L;
        gui::retained::write_field(particle->remaining, float(remaining));
        gui::retained::write_field(particle->remaining, alive ? float(remaining) : 0.f);
    }
}
// Initialization uses the same warm-up loop.
void warm_up_menu_particles(ParticleWidget *widget) {
    if (!widget->maximum_particles || widget->spawn_rate == 0 || widget->life == 0)
        return;
    const float seconds = menu_particle_period(widget->spawn_rate);
    float accumulated = 0;
    auto retained_accumulated = number(accumulated);
    auto duration = number(widget->life) * widget->spawn_rate;
    for (int count = 0; retained_accumulated < duration;) {
        spawn_menu_particle(widget);
        advance_menu_particles(widget, seconds);
        if (widget->maximum_particles <= ++count)
            return;
        retained_accumulated = number(accumulated) + seconds;
        accumulated = float(retained_accumulated);
        duration = number(widget->life) * widget->spawn_rate;
    }
}
} // namespace nfs::application::frontend
