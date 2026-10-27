#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/resources/ini/ini.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include <cstdio>
#include <cstring>

namespace nfs::graphics::startup {
// Missing INI fields retain the existing Reader contract;
// no new fallback defaults are introduced for an existing file.
void read_render_settings() {
    char path[272];
    std::sprintf(path, "%s\\rendercaps.ini", configuration_path());
    if (!files::file::exists(path)) {
        settings.fullscreen = 1;
        settings.race.index = settings.frontend.index = 2147483647;
        settings.gamma = -1;
        return;
    }
    ini::Reader reader;
    ini::reader_construct(&reader, path, 0);
    try {
        ini::get_int(&reader, "Graphics", "Fullscreen",
                     reinterpret_cast<int *>(&settings.fullscreen));
        ini::get_int(&reader, "Graphics", "LimitResolution",
                     reinterpret_cast<int *>(&settings.limit_resolution));
        ini::get_float(&reader, "Graphics", "GammaScale", &settings.gamma);
        ini::get_int(&reader, "Graphics", "MipMap", reinterpret_cast<int *>(&settings.mipmap));
        const char *names[] = {"ScreenModeIndex", "BDepth", "ZDepth", "Stencil", "Width", "Height"};
        for (unsigned i = 0; i < 6; ++i)
            ini::get_int(&reader, "Graphics", names[i],
                         reinterpret_cast<int *>(&settings.race) + i);
        for (unsigned i = 0; i < 6; ++i)
            ini::get_int(&reader, "GraphicsFE", names[i],
                         reinterpret_cast<int *>(&settings.frontend) + i);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

void initialize_render_settings() {
    settings_owner = lifecycle::allocate(1, "RENDER CAPABILITIES", gui::application_pool);
    if (settings_owner)
        read_render_settings();
}

void shutdown_render_settings() {
    if (settings_owner)
        gui::application_release(settings_owner);
    settings_owner = nullptr;
}

void write_render_settings() {
    char path[272];
    std::sprintf(path, "%s\\rendercaps.ini", configuration_path());
    ini::Reader reader;
    ini::reader_construct(&reader, path, 0);
    try {
        ini::set_int(&reader, "Graphics", "Fullscreen",
                     !settings.supports_desktop || settings.fullscreen != 0);
        ini::set_int(&reader, "Graphics", "LimitResolution", settings.limit_resolution);
        ini::set_float(&reader, "Graphics", "GammaScale", settings.gamma, 6);
        ini::set_int(&reader, "Graphics", "MipMap", settings.mipmap);
        const char *names[] = {"ScreenModeIndex", "BDepth", "ZDepth", "Stencil", "Width", "Height"};
        for (unsigned i = 0; i < 6; ++i)
            ini::set_int(&reader, "Graphics", names[i],
                         reinterpret_cast<const int *>(&settings.race)[i]);
        for (unsigned i = 0; i < 6; ++i)
            ini::set_int(&reader, "GraphicsFE", names[i],
                         reinterpret_cast<const int *>(&settings.frontend)[i]);
    } catch (...) {
        ini::reader_clear(&reader);
        throw;
    }
    ini::reader_clear(&reader);
}

} // namespace nfs::graphics::startup
