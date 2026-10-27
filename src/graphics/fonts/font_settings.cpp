#include "nfs/graphics/fonts/font_faces.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::fonts::faces {

void read_settings(Manager *manager, ini::Reader *reader) {
    int face_index = 0;
    for (int i = 0; i < 10; ++i) {
        char filename[260], section[32];
        filename[0] = 0;
        std::sprintf(section, "font%d", i);
        int patchwork = 0;
        ini::get_string(reader, section, "fontfile", filename, 259);
        ini::get_int(reader, section, "patchwork", &patchwork);
        if (filename[0]) {
            if (face_index < manager->count) {
                std::strcat(filename, ".ffn");
                Face *face = manager->faces + face_index;
                std::strcpy(face->filename, filename);
                int when = 0;
                ini::get_int(reader, section, "when", &when);
                face->when = when;
                face->use_patchwork = patchwork != 0;
            }
            ++face_index;
        }
    }
}

int count_settings(ini::Reader *reader) {
    int count = 0;
    for (int i = 0; i < 10; ++i) {
        char filename[260], section[32];
        filename[0] = 0;
        std::sprintf(section, "font%d", i);
        ini::get_string(reader, section, "fontfile", filename, 259);
        if (filename[0])
            ++count;
    }
    return count;
}
} // namespace nfs::fonts::faces
