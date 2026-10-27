#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/resources/ini/ini.hpp"
#include <cstdio>
namespace nfs::lifecycle::runtime {
using namespace game::runtime;

void read_world_lights(Dword *object) {
    char path[260];
    std::sprintf(path, level_file("lighting.ini"));
    ini::Reader reader;
    ini::reader_construct(&reader, path, 0);
    try {
        auto colour = [&](const char *section, unsigned offset, bool alpha) {
            int red = 0, green = 0, blue = 0, opacity = 255;
            ini::get_int(&reader, section, "red", &red);
            ini::get_int(&reader, section, "green", &green);
            ini::get_int(&reader, section, "blue", &blue);
            if (alpha)
                ini::get_int(&reader, section, "alpha", &opacity);
            at<float>(object, offset) =
                float(static_cast<long double>(red) * 0.0039215688593685626983642578125L);
            at<float>(object, offset + 4) =
                float(static_cast<long double>(green) * 0.0039215688593685626983642578125L);
            at<float>(object, offset + 8) =
                float(static_cast<long double>(blue) * 0.0039215688593685626983642578125L);
            if (alpha)
                at<float>(object, offset + 12) =
                    float(static_cast<long double>(opacity) * 0.0039215688593685626983642578125L);
        };
        colour("ambientsky", 12, false);
        colour("ambientground", 24, false);
        colour("sun", 76, false);
        if (at<int>(system(24), 76) < 1) {
            ini::get_float(&reader, "sun", "position_x", &at<float>(object, 36));
            ini::get_float(&reader, "sun", "position_y", &at<float>(object, 40));
            ini::get_float(&reader, "sun", "position_z", &at<float>(object, 44));
        } else {
            void *marker = level_marker(system(24), 4, 0);
            at<Dword>(object, 36) = at<Dword>(marker, 0);
            at<Dword>(object, 40) = at<Dword>(marker, 4);
            at<float>(object, 44) = -at<float>(marker, 8);
        }
        ini::get_float(&reader, "sun", "carposition_x", &at<float>(object, 64));
        ini::get_float(&reader, "sun", "carposition_y", &at<float>(object, 68));
        ini::get_float(&reader, "sun", "carposition_z", &at<float>(object, 72));
        const auto variant = network::packets::rule_value(system(72), 6);
        if (variant == 2 || network::packets::rule_value(system(72), 6) == 3) {
            at<float>(object, 64) = -at<float>(object, 64);
            at<float>(object, 36) = -at<float>(object, 36);
        }
        normalize_vector(reinterpret_cast<float *>(object + 64 / 4),
                         reinterpret_cast<float *>(object + 64 / 4));
        colour("shadowcolour", 48, true);
        float direction[3];
        normalize_vector(reinterpret_cast<float *>(object + 36 / 4), direction);
        for (unsigned i = 0; i < 3; ++i)
            at<float>(object, 88 + i * 4) = direction[i];
        at<Dword>(object, 100) = 1065353216;
        at<Byte>(object, 4) = 1;
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}
} // namespace nfs::lifecycle::runtime
namespace nfs::lifecycle::table_entries {

void __fastcall load_world_lights(System *object, void *) {
    runtime::reset_world_lights(reinterpret_cast<kernel::Dword *>(object));
    runtime::read_world_lights(reinterpret_cast<kernel::Dword *>(object));
}
} // namespace nfs::lifecycle::table_entries
