#include "nfs/graphics/transforms/transform_handlers.hpp"
namespace nfs::game::runtime::physics {
int contact_iterations = 32;
const float pair_friction[2] = {0.5f, 0.430000007152557373046875f};
} // namespace nfs::game::runtime::physics
namespace nfs::game::runtime::matrices::handlers {
float near_plane = 1.0f;
float depth_scale = 0.0020039998926222324371337890625f;
float screen_x = 319.5f;
float screen_y = 319.5f;
float offset_x = 319.5f;
float offset_y = 239.5f;
float reciprocal_scale = 1.0f;
Dword projection_changed = 1;
Dword clip_intersection = 255, clip_union{};
void *clip_cursor{}, *copy_cursor{}, *repeat_cursor{}, *repeat_flags_cursor{};
alignas(16) float null_translation[4]{};
Dword clip_limits[8]{};
} // namespace nfs::game::runtime::matrices::handlers
namespace nfs::sound::pending {
const Dword default_envelope[2] = {2147483647, 127};
const std::uint16_t pan_lookup[128] = {
    49152, 49159, 49174, 49199, 49234, 49278, 49333, 49397, 49471, 49554, 49647, 49750, 49862,
    49984, 50115, 50255, 50404, 50563, 50730, 50907, 51092, 51286, 51489, 51700, 51919, 52147,
    52382, 52626, 52877, 53136, 53402, 53676, 53957, 54244, 54539, 54840, 55148, 55462, 55781,
    56107, 56439, 56776, 57118, 57465, 57817, 58174, 58535, 58901, 59270, 59643, 60020, 60400,
    60783, 61169, 61558, 61949, 62342, 62737, 63134, 63532, 63931, 64332, 64733, 65135, 0,
    408,   816,   1223,  1630,  2036,  2440,  2843,  3244,  3643,  4040,  4434,  4826,  5215,
    5600,  5982,  6360,  6734,  7104,  7470,  7831,  8187,  8538,  8884,  9224,  9559,  9887,
    10210, 10526, 10835, 11138, 11434, 11723, 12004, 12279, 12545, 12804, 13054, 13297, 13531,
    13757, 13975, 14183, 14383, 14574, 14756, 14929, 15092, 15246, 15391, 15526, 15652, 15767,
    15873, 15969, 16055, 16132, 16198, 16254, 16300, 16336, 16362, 16377, 16383};
} // namespace nfs::sound::pending
namespace nfs::game::runtime::physics::pending {
const Dword support_triangles[64 * 3] = {
    18, 1,  13, 1,  19, 13, 19, 2,  13, 2,  21, 13, 21, 3,  13, 3,  20, 13, 20, 0,  13, 0,  18, 13,
    5,  14, 12, 14, 4,  12, 4,  16, 12, 16, 7,  12, 7,  17, 12, 17, 6,  12, 6,  15, 12, 15, 5,  12,
    4,  14, 8,  14, 5,  8,  5,  1,  8,  1,  18, 8,  18, 0,  8,  0,  4,  8,  6,  17, 9,  17, 7,  9,
    7,  3,  9,  3,  21, 9,  21, 2,  9,  2,  6,  9,  5,  15, 11, 15, 6,  11, 6,  2,  11, 2,  19, 11,
    19, 1,  11, 1,  5,  11, 7,  16, 10, 16, 4,  10, 4,  0,  10, 0,  20, 10, 20, 3,  10, 3,  7,  10,
    0,  4,  5,  5,  1,  0,  5,  6,  2,  5,  2,  1,  6,  7,  3,  6,  3,  2,  7,  4,  0,  7,  0,  3,
    0,  1,  2,  0,  2,  3,  5,  4,  7,  5,  7,  6,  0,  1,  3,  1,  2,  3,  12, 1,  0,  12, 2,  1,
    12, 3,  2,  12, 0,  3,  13, 4,  5,  5,  6,  13, 6,  7,  13, 7,  4,  13, 5,  4,  7,  5,  7,  6};
const char shape_source_path[] =
    "C:\\HP2\\Net\\Patch1\\Code\\Source\\Sim\\Physics\\COLLIDE.c";
const char shape_error_message[] = "Undefined Object Shape";
} // namespace nfs::game::runtime::physics::pending
