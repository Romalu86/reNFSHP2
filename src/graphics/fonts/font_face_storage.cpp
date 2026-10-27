#include "nfs/graphics/fonts/font_faces.hpp"
#include <cstring>
namespace nfs::fonts::faces {

void *construct(void *storage) {
    auto *face = static_cast<Face *>(storage);
    face->filename[0] = 0;
    face->file = face->font = nullptr;
    face->patchwork = nullptr;
    face->style_3c = 0;
    face->style_34 = face->style_38 = 1;
    face->style_2c = face->style_30 = 0;
    face->size_ratio = 1;
    face->use_patchwork = 0;
    return storage;
}

void set_filename(Face *face, const char *filename) {
    std::strcpy(face->filename, filename);
}
} // namespace nfs::fonts::faces
