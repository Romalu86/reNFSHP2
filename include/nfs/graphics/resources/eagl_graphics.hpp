#pragma once
#include "nfs/graphics/backend/eagl.hpp"
#include "nfs/graphics/fonts/fonts.hpp"
#include "nfs/core/lifecycle/kernel_lifecycle.hpp"
namespace nfs::graphics {
using kernel::Dword;
struct Device {Dword* limits;Dword fields[13];};
struct TextureReader {Dword fields[50];};
static_assert(sizeof(Device)==56&&sizeof(TextureReader)==200);
extern Device* device;extern Dword* context;extern Dword* main_viewport;extern Dword* ui_viewport;extern Dword* background_viewport;
extern void* camera;extern void* module_data;extern eagl::Loader* module;
extern Dword mode;extern std::uint8_t gamma_saved;extern WORD gamma[3][256];
extern Dword &server_requested, &graphics_disabled;extern void* current_device;extern Dword* current_context;
extern char allocation_name[64];
extern Dword font_shape_type;extern float camera_field_of_view;
extern Dword previous_frame_tick;
extern Dword positional_lights[28];
void initialize_positional_light_sources();
void* allocate(Dword,const char*);void release(void*,Dword);void begin_allocation(const char*,Dword);void end_allocation();
Device* construct_device(Device*);int initialize();void shutdown();eagl::Loader* load_module(const char*,eagl::Resolve,void**,Dword);
namespace unresolved {



extern Dword camera_state;
extern void*& initial_viewport;extern wchar_t error_message[];
}
}
