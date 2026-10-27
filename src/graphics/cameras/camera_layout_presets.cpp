#include "nfs/world/lifecycle/world_lifecycle.hpp"
#include "nfs/network/packets/network_packet_handlers.hpp"
#include "../../math/precision/extended_float.hpp"
namespace nfs::game::world::camera_layout {
// Scalar channel/interval loads follow the FLD/FST conversion; other setters copy words. The mode
// object's +4 subobject is the receiver of the interval setters.

void select_full_interval(void *object) {
    const float value = float(kernel::numeric::Extended(at<float>(object, 36),
                                                       kernel::numeric::current_format()));
    at<float>(object, 32) = value;
    at<float>(object, 28) = value;
}

void select_vertical_interval(void *object) {
    const float value = float(kernel::numeric::Extended(at<float>(object, 40),
                                                       kernel::numeric::current_format()));
    at<float>(object, 32) = value;
    at<float>(object, 28) = value;
}

void select_horizontal_interval(void *object) {
    const float value = float(kernel::numeric::Extended(at<float>(object, 44),
                                                       kernel::numeric::current_format()));
    at<float>(object, 32) = value;
    at<float>(object, 28) = value;
}

void select_full_vector_parameters(void *object) {
    for (unsigned index = 0; index != 10; ++index)
        at<Dword>(object, 100 + index * 4) = at<Dword>(object, 140 + index * 4);
}

void select_vertical_vector_parameters(void *object) {
    for (unsigned index = 0; index != 10; ++index)
        at<Dword>(object, 100 + index * 4) = at<Dword>(object, 180 + index * 4);
}

void select_horizontal_vector_parameters(void *object) {
    for (unsigned index = 0; index != 10; ++index)
        at<Dword>(object, 100 + index * 4) = at<Dword>(object, 220 + index * 4);
}

void select_full_three_channels(void *object) {
    const float value = float(kernel::numeric::Extended(at<float>(object, 16),
                                                       kernel::numeric::current_format()));
    at<float>(object, 4) = value;
    at<float>(object, 12) = value;
    at<float>(object, 8) = value;
}

void select_vertical_three_channels(void *object) {
    const float value = float(kernel::numeric::Extended(at<float>(object, 20),
                                                       kernel::numeric::current_format()));
    at<float>(object, 4) = value;
    at<float>(object, 12) = value;
    at<float>(object, 8) = value;
}

void select_horizontal_three_channels(void *object) {
    const float value = float(kernel::numeric::Extended(at<float>(object, 24),
                                                       kernel::numeric::current_format()));
    at<float>(object, 4) = value;
    at<float>(object, 12) = value;
    at<float>(object, 8) = value;
}

void select_full_primary_scalar(void *object) {
    at<Dword>(object, 8) = at<Dword>(object, 12);
}

void select_vertical_primary_scalar(void *object) {
    at<Dword>(object, 8) = at<Dword>(object, 16);
}

void select_horizontal_primary_scalar(void *object) {
    at<Dword>(object, 8) = at<Dword>(object, 20);
}

void select_full_secondary_scalar(void *object) {
    at<Dword>(object, 24) = at<Dword>(object, 28);
}

void select_vertical_secondary_scalar(void *object) {
    at<Dword>(object, 24) = at<Dword>(object, 32);
}

void select_horizontal_secondary_scalar(void *object) {
    at<Dword>(object, 24) = at<Dword>(object, 36);
}

void select_full_mode_three_vector(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 24);
}

void select_vertical_mode_three_vector(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 28);
}

void select_horizontal_mode_three_vector(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 32);
}

void select_full_mode_three_scalar(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 24);
}

void select_vertical_mode_three_scalar(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 28);
}

void select_horizontal_mode_three_scalar(void *object) {
    at<Dword>(object, 20) = at<Dword>(object, 32);
}
namespace {
unsigned selected_layout() {
    if (network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 13) == 1)
        return 0;
    return network::packets::rule_value(static_cast<Dword *>(subsystem(72)), 38) ? 2 : 1;
}
using Setter = void (*)(void *);
const Setter intervals[] = {select_full_interval, select_vertical_interval,
                            select_horizontal_interval};
const Setter vectors[] = {select_full_vector_parameters, select_vertical_vector_parameters,
                          select_horizontal_vector_parameters};
const Setter three_channels[] = {select_full_three_channels, select_vertical_three_channels,
                                 select_horizontal_three_channels};
const Setter primary_scalars[] = {select_full_primary_scalar, select_vertical_primary_scalar,
                                  select_horizontal_primary_scalar};
const Setter secondary_scalars[] = {select_full_secondary_scalar, select_vertical_secondary_scalar,
                                    select_horizontal_secondary_scalar};
void configure_mode(void *mode, bool has_three_channels) {
    void *primary = at<void *>(mode, 84);
    void *vector = at<void *>(mode, 80);
    void *channels = has_three_channels ? at<void *>(mode, 88) : nullptr;
    void *secondary = at<void *>(mode, 96);
    if (!primary || !vector || (has_three_channels && !channels) || !secondary)
        return;
    const unsigned layout = selected_layout();
    intervals[layout](offset(mode, 4));
    vectors[layout](vector);
    if (has_three_channels)
        three_channels[layout](channels);
    primary_scalars[layout](primary);
    secondary_scalars[layout](secondary);
}
} // namespace

void refresh_mode_one(void *owner) {
    if (void *mode = find_camera_mode(owner, 1))
        configure_mode(mode, true);
}

void refresh_mode_zero(void *owner) {
    if (void *mode = find_camera_mode(owner, 0))
        configure_mode(mode, true);
}

void refresh_mode_two(void *owner) {
    if (void *mode = find_camera_mode(owner, 2))
        configure_mode(mode, false);
}

void refresh_mode_three(void *owner) {
    if (void *mode = find_camera_mode(owner, 3)) {
        void *vector = at<void *>(mode, 80);
        void *scalar = at<void *>(mode, 84);
        unsigned layout = selected_layout();
        intervals[layout](offset(mode, 4));
        if (vector && scalar) {
            const Setter mode_vectors[] = {select_full_mode_three_vector,
                                           select_vertical_mode_three_vector,
                                           select_horizontal_mode_three_vector};
            const Setter mode_scalars[] = {select_full_mode_three_scalar,
                                           select_vertical_mode_three_scalar,
                                           select_horizontal_mode_three_scalar};
            mode_vectors[layout](vector);
            mode_scalars[layout](scalar);
        }
    }
}
} // namespace nfs::game::world::camera_layout
namespace nfs::application::pending {

void refresh_camera_layout(void *owner) {
    game::world::camera_layout::refresh_mode_one(owner);
    game::world::camera_layout::refresh_mode_zero(owner);
    game::world::camera_layout::refresh_mode_two(owner);
    game::world::camera_layout::refresh_mode_three(owner);
}
} // namespace nfs::application::pending
