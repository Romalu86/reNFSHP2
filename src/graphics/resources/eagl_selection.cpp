#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::graphics::backend {

Dword* selected_context(Device*){return current_context;}

Device* selected_device(){return static_cast<Device*>(current_device);}
}
