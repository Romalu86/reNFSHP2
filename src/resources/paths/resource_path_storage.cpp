#include "nfs/world/resources/runtime_resources.hpp"
#include "nfs/graphics/configuration/display_configuration.hpp"
namespace nfs::graphics::startup {

// strcpy producers retain their saved unchecked behavior; this is not a new length limit.
char display_caption[256]{};

char configuration_buffer[260]{};
} // namespace nfs::graphics::startup
namespace nfs::game::runtime::pending {

 
char level_path[264]{};
char track_path[264]{};
} // namespace nfs::game::runtime::pending
