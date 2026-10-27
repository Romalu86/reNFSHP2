#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
#include "nfs/core/configuration/launch_options.hpp"
namespace nfs::game::runtime::ai::lifecycle::pending {
Dword &disable_ai =
    *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + (8627836 - 8627648));
Dword &explicit_car_kind =
    *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + (8627840 - 8627648));
Dword &explicit_car_skin =
    *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + (8627696 - 8627648));
Dword &explicit_car_variant =
    *reinterpret_cast<Dword *>(reinterpret_cast<Byte *>(&launch::options) + (8627852 - 8627648));
char (&car_kind_text)[24] = *reinterpret_cast<char (*)[24]>(
    reinterpret_cast<Byte *>(&launch::options) + (8629108 - 8627648));
char (&car_skin_text)[24] = *reinterpret_cast<char (*)[24]>(
    reinterpret_cast<Byte *>(&launch::options) + (8628244 - 8627648));
char (&car_variant_text)[24] = *reinterpret_cast<char (*)[24]>(
    reinterpret_cast<Byte *>(&launch::options) + (8629180 - 8627648));
} // namespace nfs::game::runtime::ai::lifecycle::pending
