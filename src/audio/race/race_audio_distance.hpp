#pragma once

namespace nfs::game::runtime::race_audio::pending {
// Private primitive interface: component and distance F32 stores precede current rounding.
int rounded_point_distance(const float *first, const float *second);
}
