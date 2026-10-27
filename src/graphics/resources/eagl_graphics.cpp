#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include "nfs/graphics/resources/eagl_objects.hpp"
#include "nfs/graphics/resources/eagl_cleanup.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/core/platform/cpu.hpp"
#include "nfs/graphics/configuration/render_settings.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/graphics/resources/eagl_camera.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "nfs/graphics/display/viewport_transform.hpp"
#include "nfs/graphics/primitives/immediate_draw.hpp"
#include "nfs/graphics/fonts/font_renderer.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
#include "nfs/graphics/display/graphics_error.hpp"
#include "nfs/core/frame/game_step.hpp"
#include "nfs/physics/solver/solver_runtime.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::graphics {
static Dword allocation_sequence = 0, previous_pool = 0;
char allocation_name[64]{};
static Dword z_bias[4]{};
static void *direct3d_interface = nullptr;
static void *direct3d_device = nullptr;
Dword previous_frame_tick = 0;

void *allocate(Dword bytes, const char *name) {
    char label[260];
    std::sprintf(label, lifecycle::current_pool == gui::application_pool ? "%d %s" : "H %d %s",
                 allocation_sequence, name);
    ++allocation_sequence;
    return kernel::memory_allocate(label, bytes, lifecycle::current_pool);
}

void release(void *memory, Dword) {
    if (memory)
        kernel::memory_free(memory);
}

void begin_allocation(const char *name, Dword pool) {
    std::strncpy(allocation_name, name, 63);
    allocation_name[63] = 0;
    previous_pool = kernel::default_pool;
    kernel::default_pool = pool;
}
void end_allocation() {
    allocation_name[0] = 0;
    kernel::default_pool = previous_pool;
}

Device *construct_device(Device *value) {
    value->limits = reinterpret_cast<Dword *>(value);
    Dword *object = reinterpret_cast<Dword *>(value);
    object[1] = 0;
    object[11] = object[12] = 0;
    reinterpret_cast<std::uint8_t *>(object)[52] = 0;
    current_device = value;
    object[4] = object[5] = 16384;
    object[1] = 0;
    object[6] = object[7] = 255;
    object[9] = 32000;
    object[8] = 1024;
    object[11] = object[12] = 0;
    current_context = nullptr;
    reinterpret_cast<std::uint8_t *>(object)[40] = 0;
    kernel::cpu::initialize();
    return value;
}
 
eagl::Loader *load_module(const char *path, eagl::Resolve resolve, void **data, Dword pool) {
    if (!device)
        return nullptr;
    void *image = files::file::try_load(path, pool);
    if (!image)
        return nullptr;
    auto *loader = static_cast<eagl::Loader *>(lifecycle::allocate(28, "DynamicLoader", pool));
    if (loader) {
        try {
            loader = eagl::loader_construct(loader, static_cast<eagl::ElfHeader *>(image),
                                            kernel::memory_size(image), resolve);
        } catch (...) {
            game::runtime::track::pending::release_named(loader, "DynamicLoader", pool);
            throw;
        }
    }
    if (!loader)
        return nullptr;
    if (data)
        *data = image;
    return loader;
}
 
int initialize() {
    using namespace unresolved;
    HWND desktop = GetDesktopWindow();
    HDC dc = GetWindowDC(desktop);
    int previous_width = GetDeviceCaps(dc, 8), previous_height = GetDeviceCaps(dc, 10),
        previous_depth = GetDeviceCaps(dc, 12);
    ReleaseDC(desktop, dc);
    if (!gamma_saved) {
        desktop = GetDesktopWindow();
        dc = GetWindowDC(desktop);
        BOOL saved = GetDeviceGammaRamp(dc, gamma);
        ReleaseDC(desktop, dc);
        if (saved)
            gamma_saved = 1;
    }
    eagl::allocator = allocate;
    eagl::deallocator = release;
    if (server_requested && graphics_disabled) {
        device = nullptr;
        return 0;
    }
    auto *storage =
        static_cast<Device *>(lifecycle::allocate(56, "gEAGLDev", gui::application_pool));
    if (storage) {
        try {
            storage = construct_device(storage);
        } catch (...) {
            game::runtime::track::pending::release_named(
                storage, "gEAGLDev", gui::application_pool);
            throw;
        }
    }
    device = storage;
    device->limits[6] = 7500;
    device->limits[7] = 7500;  
    begin_allocation("gEAGLDev_Init", gui::application_pool);
    if (!backend::initialize_device(device)) {
        DWORD error = 0;
        if (current_context)
            error = *current_context;
        if ((error & 65535) == 8 || (error & 65535) == 14)
            return 3;
        wchar_t *message;
        if (error && FormatMessageW(4864, nullptr, error, 1024,
                                    reinterpret_cast<LPWSTR>(&message), 0, nullptr)) {
            wsprintfW(error_message, L"%s", message);
            LocalFree(message);
        }
        return 1;
    }
    end_allocation();
    direct3d_interface = backend_interface;
    desktop = GetDesktopWindow();
    dc = GetWindowDC(desktop);
    int width = GetDeviceCaps(dc, 8), height = GetDeviceCaps(dc, 10), depth = GetDeviceCaps(dc, 12);
    ReleaseDC(desktop, dc);
    if (width != previous_width || height != previous_height || depth != previous_depth)
        ChangeDisplaySettingsA(nullptr, 0);
    desktop = GetDesktopWindow();
    dc = GetWindowDC(desktop);
    width = GetDeviceCaps(dc, 8);
    height = GetDeviceCaps(dc, 10);
    depth = GetDeviceCaps(dc, 12);
    ReleaseDC(desktop, dc);
    context = create_context(device);
    set_context_mode(context, 0);
    startup::initialize_render_settings();
    Dword *render_context = reinterpret_cast<Dword *>(context[1]);
    render_context[188 / 4] = reinterpret_cast<Dword>(suspend_rendering);  
    mode = 1;
    const char *name = reinterpret_cast<const char *>(render_context[180 / 4]);
    if (name)
        std::strcpy(startup::display_caption, name);  
    startup::display_caps = backend::adapter_caps();
    startup::settings.supports_desktop = (startup::display_caps[560 / 4] & 524288) != 0;
    if ((depth != 16 && depth != 32) || width < 801 || height < 601)
        startup::settings.supports_desktop = 0;
    char texture_path[272];
    std::sprintf(texture_path, "%s.", startup::configuration_path());
    TextureReader reader;
    textures::conversion_enabled = 0;
    textures::construct(&reader, "TexConv.ini", texture_path);
    try {
        textures::device_settings(&textures::database);
        textures::write_settings(&textures::database, "TexConv.ini", texture_path);
        if (reader.fields[2] == 2 || textures::different(&textures::database, &reader))
            textures::conversion_enabled = 1;
        font_shape_type = 109;
        float display_width, display_height;
        std::uint8_t display_flag;
        int result = startup::configure_display(1, &display_width, &display_height, &display_flag);
        if (result) {
            textures::destroy(&reader);
            return result;
        }
        direct3d_device = backend_device;
        begin_allocation("myEAGLFontInit", lifecycle::ui_pool);
        fonts::set_callbacks(&font_renderer::callbacks);
        end_allocation();
        main_viewport = create_viewport(context);
        viewports::rectangle(main_viewport, 0, 0, display_width, display_height, 0,
                             1);  
        camera_field_of_view = 50;
        background_viewport = create_viewport(context);
        viewports::rectangle(background_viewport, 0, 0, display_width, display_height, 0, 1);
        viewports::orthographic(background_viewport, 1, 1, 1300);
        background_viewport[12] = 4282010938u;  
        ui_viewport = create_viewport(context);
        viewports::rectangle(ui_viewport, 0, 0, display_width, display_height, 0, 1);
        float scale[16] = {
            2.0f / display_width, 0, 0, 0, 0, -2.0f / display_height, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
        float offset[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -1, 1, 0, 1};
        math::append(scale, offset);
        viewports::set_matrix(background_viewport, scale);
        float view[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -5, 1};
        viewports::set_matrix(main_viewport, view);
        bindings::initialize();
        z_bias[0] = z_bias[1] = z_bias[3] = 0;
        z_bias[2] = 1120403456;
        eagl::pool_register(&eagl::external_symbols, "gpzbias", reinterpret_cast<Dword>(z_bias));
        eagl::pool_register(&eagl::external_symbols, "gpPositionalLightSources",
                            reinterpret_cast<Dword>(positional_lights));
        eagl::pool_register(&eagl::external_symbols, "EAGLAnimationBuffer",
                            reinterpret_cast<Dword>(lifecycle::runtime::animation_buffer));
        eagl::pool_register(&eagl::external_symbols, "gTimeFactor",
                            reinterpret_cast<Dword>(&game::simulation_time));
        char module_path[264];
        std::sprintf(module_path, "%seaglrm.o",
                     reinterpret_cast<const char *>(lifecycle::manager->slots[1]) + 264);
        module =
            load_module(module_path, symbols::resolve_module, &module_data, gui::application_pool);
        const float eye[] = {0, 0, 50, 1}, target[] = {0, 0, 0, 1}, up[] = {0, 1, 0, 0};
        auto *object =
            static_cast<Dword *>(lifecycle::allocate(172, "gEAGLCamera", gui::application_pool));
        if (object) {
            object[17] = 0;
            *reinterpret_cast<const gui::Destroy **>(object) = cameras::base_methods;
            try {
                *reinterpret_cast<const gui::Destroy **>(object) = cameras::camera_methods;
                cameras::look_at(reinterpret_cast<cameras::Camera *>(object), eye, target, up);
            } catch (...) {
                *reinterpret_cast<const gui::Destroy **>(object) = cameras::base_methods;
                game::runtime::track::pending::release_named(
                    object, "gEAGLCamera", gui::application_pool);
                throw;
            }
        }
        camera = object;
        previous_frame_tick = camera_state;
        initial_viewport = main_viewport;
    } catch (...) {
        textures::destroy(&reader);
        throw;
    }
    textures::destroy(&reader);
    return 0;
}
// The saved gamma flag is deliberately not reset.
void shutdown() {
    using namespace unresolved;
    bindings::shutdown();
    mode = 2;
    if (camera)
        (*reinterpret_cast<const gui::Destroy **>(camera))[0](camera, 1);
    camera = nullptr;
    // Display compatibility: loader destructors still receive addresses in this image.
    if (module) {
        eagl::loader_destroy(module);
        eagl::deallocator(module, 28);
    }
    module = nullptr;
    if (ui_viewport)
        destroy_viewport(context, ui_viewport);
    ui_viewport = nullptr;
    if (background_viewport)
        destroy_viewport(context, background_viewport);
    background_viewport = nullptr;
    if (main_viewport)
        destroy_viewport(context, main_viewport);
    main_viewport = nullptr;
    startup::shutdown_render_settings();
    if (context)
        destroy_context(device, context);
    context = nullptr;
    if (device) {
        device_shutdown(device);
        eagl::deallocator(device, 56);
    }
    device = nullptr;
    // Context/device cleanup can still visit render definitions from the image.
    kernel::memory_free(module_data);
    module_data = nullptr;
    if (gamma_saved) {
        HWND desktop = GetDesktopWindow();
        HDC dc = GetWindowDC(desktop);
        SetDeviceGammaRamp(dc, gamma);
        ReleaseDC(desktop, dc);
    }
}
} // namespace nfs::graphics
