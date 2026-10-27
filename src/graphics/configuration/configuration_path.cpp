#include "nfs/graphics/configuration/render_settings.hpp"
#include <cstring>

namespace nfs::graphics::startup {
// The option flag and24-byte value share the launch Options owner.

// The custom variant removes the path's final byte, then

const char *configuration_path() {
    const char *root = reinterpret_cast<const char *>(lifecycle::manager->slots[1]) + 3124;
    if (!configuration_variant_enabled)
        return root;
    std::strcpy(configuration_buffer, root);
    configuration_buffer[std::strlen(root) - 1] = 0;
    std::strcat(configuration_buffer, configuration_variant);
    std::strcat(configuration_buffer, "\\");
    return configuration_buffer;
}
} // namespace nfs::graphics::startup
