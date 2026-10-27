#include "nfs/graphics/fonts/font_faces.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/resources/localization/region.hpp"
#include "nfs/resources/files/file_cache.hpp"
#include "../../gui/lifecycle/gui_render_numeric_x87.hpp"
#include <cctype>
#include <cstdio>
#include <cstring>

namespace gui_numeric = ::nfs::gui::retained;
namespace nfs::fonts::faces {

void draw(Face *face, kernel::Dword *viewport, float x, float y, const char *text) {
    if (face->font) {
        if (face->patchwork) {
            patchwork::draw_text(face->patchwork, face->font, text, viewport, x, y,
                                 face->font->scale_x, face->font->scale_y, face->font->unknown_20);
            return;
        }
        fonts::draw_text(face->font, x, y, reinterpret_cast<const Byte *>(text));
    }
}


void destroy(void *storage) {
    auto *face = static_cast<Face *>(storage);
    if (!face->file && !face->font)
        return;
    if (face->patchwork) {
        patchwork::destroy(face->patchwork);
        gui::application_release(face->patchwork);
        face->patchwork = nullptr;
    } else if (face->font) {
        release_font(face->font);
    }
    face->font = nullptr;
    if (face->file) {
        kernel::memory_free(face->file);
        face->file = nullptr;
    }
}

namespace {
bool fits_texture(Font *font) {
    auto *shape = reinterpret_cast<const std::int16_t *>(reinterpret_cast<const Byte *>(font) +
                                                         font->unknown_1c);
    return static_cast<kernel::Dword>(shape[2]) <= graphics::textures::database.fields[4] &&
           static_cast<kernel::Dword>(shape[3]) <= graphics::textures::database.fields[5];
}
} // namespace

void load(Face *face) {
    if (!face->filename[0])
        return;
    graphics::begin_allocation("LoadFont", lifecycle::ui_pool);
    if (!face->file) {
        char path[260], candidate[260];
        const char *directory =
            reinterpret_cast<const char *>(lifecycle::manager->slots[1]) + 2344;
        if (region::get() == 6) {
            std::sprintf(path, "%s%s%s", directory, "Korean\\", face->filename);
        } else {
            face->use_patchwork = 0;
            std::sprintf(path, "%s%s", directory, face->filename);
        }
        std::strcpy(candidate, path);
        int position = static_cast<int>(std::strlen(candidate)) - 1;
        while (position >= 0 && !std::isdigit(static_cast<Byte>(candidate[position])))
            --position;
        char *digits = nullptr;
        int digit_count = 0, requested_size = 0;
        if (position >= 0) {
            if (position != 0) {
                while (position >= 0 && std::isdigit(static_cast<Byte>(candidate[position])))
                    --position;
                digits = candidate + position + 1;
            } else
                digits = candidate;
            while (std::isdigit(static_cast<Byte>(digits[digit_count])))
                ++digit_count;
            if (digit_count)
                std::sscanf(digits, "%u", reinterpret_cast<unsigned *>(&requested_size));
        }
        auto *file =
            static_cast<Font *>(files::file::try_load(candidate, lifecycle::ui_pool | 1024));
        // EDI retains the face address on the rejected, non-two-digit path.
        kernel::Dword loaded_size = reinterpret_cast<kernel::Dword>(face);
        if (file) {
            if (!face->use_patchwork && !fits_texture(file)) {
                kernel::memory_free(file);
                file = nullptr;
                if (digit_count == 2 && requested_size != 0) {
                    loaded_size = static_cast<kernel::Dword>(requested_size);
                    while (--loaded_size > 1) {
                        char replacement[32];
                        std::sprintf(replacement, "%02u", loaded_size);
                        digits[0] = replacement[0];
                        digits[1] = replacement[1];
                        file = static_cast<Font *>(
                            files::file::try_load(candidate, lifecycle::ui_pool | 1024));
                        if (file) {
                            if (fits_texture(file))
                                break;
                            kernel::memory_free(file);
                            file = nullptr;
                        }
                    }
                }
            } else {
                if (requested_size == 0)
                    requested_size = 20;
                loaded_size = static_cast<kernel::Dword>(requested_size);
            }
        }
        face->file = file;
        if (file) {

            face->size_ratio = float(gui_numeric::integer(std::int64_t(loaded_size)) /
                                     gui_numeric::integer(requested_size));
        }
    }
    if (face->file && !face->font) {
        begin_font_pool();
        if (!face->use_patchwork)
            face->font = initialize_font(face->file);
        else {
            face->font = face->file;
            auto *storage = static_cast<patchwork::Cache *>(
                lifecycle::allocate(324, "patchworkfont", lifecycle::ui_pool));
            face->patchwork =
                storage ? patchwork::construct(storage, face->font, 256, 256, 100) : nullptr;
        }
        end_font_pool();
    }
    graphics::end_allocation();
}

} // namespace nfs::fonts::faces
