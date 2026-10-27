#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::backend::unresolved {
extern Dword vertex_shader,vertex_values,vertex_extra,vertex_parameters[3];

void buffer_bindings(){for(int i=0;i<8;++i)texture_filters[6][i]=~0u;for(int i=0;i<8;++i)texture_filters[5][i]=~0u;
    for(int i=0;i<8;++i)texture_objects[i]=0;for(int row=4;row>=0;--row)for(int i=0;i<8;++i)texture_filters[row][i]=~0u;for(int i=0;i<8;++i)texture_values[i]=0;}

void render_bindings(){for(Dword& value:material_state)value=0;render_dirty=1;for(Dword& value:render_states)value=~0u;depth_write_state=4294967294u;}

void texture_state() {
    bind_pixel_shader(0);
}

void vertex_state(){invalidate_pixel_shader_cache();for(Dword& value:resources::bound_streams)value=0;vertex_shader=vertex_values=resources::bound_index=vertex_extra=0;for(Dword& value:vertex_parameters)value=~0u;com(backend_device,200,Dword(60),Dword(~0u));}

void buffers_lost(){for(Dword* link=texture_links;link;link=reinterpret_cast<Dword*>(link[3]))if(link[0]){
    Dword* texture=reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(link[0])[9]);if(texture[6]){com(reinterpret_cast<Dword*>(texture[6]),8);reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(link[0])[9])[6]=0;}
    texture=reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(link[0])[9]);if(texture[7]){com(reinterpret_cast<Dword*>(texture[7]),8);reinterpret_cast<Dword*>(reinterpret_cast<Dword*>(link[0])[9])[7]=0;}}}

void buffers_restored(){for(Dword* link=texture_links;link;link=reinterpret_cast<Dword*>(link[3])){Dword* item=reinterpret_cast<Dword*>(link[0]);if(item){
    if(link[1]&1){Dword* texture=reinterpret_cast<Dword*>(item[9]);com(backend_device,80,texture[1],texture[2],texture[3],Dword(1),texture[5],Dword(0),texture+6);}
    if(link[1]&2){Dword samples;state::read_context_multisample_type(selected_context(selected_device()),&samples);Dword* texture=reinterpret_cast<Dword*>(item[9]);com(backend_device,104,texture[1],texture[2],texture[5],samples,texture+7);}}}}
}
namespace nfs::graphics::backend {
namespace {
Dword *pixel_binding_device = nullptr;
bool pixel_binding_known = false;
}

void invalidate_pixel_shader_cache() noexcept {
    // The native invalidator clears its cached handle without unbinding the driver.
    // A subsequent fixed-function request must therefore synchronize once.
    pixel_binding_known = false;
}

bool bind_pixel_shader(Dword requested) {
    Dword *device = backend_device;
    if (pixel_binding_device != device) {
        pixel_binding_device = device;
        pixel_binding_known = false;
    }
    if (!device)
        return false;
    Dword &cached = unresolved::vertex_shader;
    if (!pixel_binding_known && requested == 0) {
        Dword actual = 0;
        const int query = com(device, 356, &actual);
        if (backend_device != device) {
            pixel_binding_known = false;
            return false;
        }
        if (query >= 0 && backend_device == device) {
            cached = actual;
            pixel_binding_known = true;
        }
    }
    if (pixel_binding_known && cached == requested)
        return true;
    const int result = com(device, 352, requested);
    if (result >= 0 && backend_device == device) {
        cached = requested;
        pixel_binding_known = true;
        return true;
    }
    pixel_binding_known = false;
    return false;
}
}
