#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::textures {

Dword device_settings(TextureReader* object){if(!device)return 3;Dword* caps=backend::adapter_caps();object->fields[4]=caps[636/4];object->fields[5]=caps[640/4];
 reinterpret_cast<std::uint8_t*>(object)[13]=(caps[608/4]&32)!=0;reinterpret_cast<std::uint8_t*>(object)[12]=(caps[608/4]&2)!=0;
 std::memcpy(object->fields+14,caps+6912/4,144);select_fallbacks(object);object->fields[2]=0;return 0;}
}
