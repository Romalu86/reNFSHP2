#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::backend {

Dword* adapter_caps(){bool alternate=alternate_device(selected_device())!=0;
    if(!alternate_caps||!hardware_caps){if((alternate&&!alternate_caps)||(!alternate&&!hardware_caps))if(!retrieve_adapter_caps(alternate?1:0))return nullptr;}
    if(alternate&&alternate_caps)return alternate_caps;if(backend_device&&selected_context(selected_device())&&(!alternate||alternate_caps)){
        Dword selected=state::context_mode(selected_context(selected_device()));if(selected==1||selected==3)return alternate_caps;if(selected>3)return nullptr;}return hardware_caps;}

void detect_vertex_programs(Dword*){Dword* caps=adapter_caps();Dword features=caps[576/4];if((features&524288)!=524288||(features&65536)!=65536||(features&1048576)!=1048576)return;
    Dword id=adapter_caps()[516/4];if(id==282984817||id==268586071||id==268587351||id==1395888386){state::hardware_vertex_programs_supported=0;return;}state::hardware_vertex_programs_supported=reinterpret_cast<std::uint8_t*>(caps)[745]!=0;}

void detect_fixed_function_vertex_processing(Dword*){Dword* caps=adapter_caps();if((caps[576/4]&524288)!=524288){state::hardware_fixed_function_processing_supported=0;return;}state::hardware_fixed_function_processing_supported=1;if((caps[576/4]&65536)!=65536)state::hardware_fixed_function_processing_supported=0;}}
