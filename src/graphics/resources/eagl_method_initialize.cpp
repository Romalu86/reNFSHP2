#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/memory/memory_primitives.hpp"
namespace nfs::graphics::methods {

void initialize(Dword *method) {
    if (method[4] && !method[6]) {
        if (!reinterpret_cast<Dword *>(method[4])[3]) {
            auto *dictionary = static_cast<Dword *>(eagl::allocator(36, "EAGL::ETTObject new"));
            if (dictionary)
                techniques::object_construct(dictionary,
                    reinterpret_cast<Dword *>(reinterpret_cast<Dword *>(method[4])[2]));
            reinterpret_cast<Dword *>(method[4])[3] = reinterpret_cast<Dword>(dictionary);
        }
        method[3] = reinterpret_cast<Dword *>(method[4])[3];
    } else if (!method[3]) {
        auto *dictionary = static_cast<Dword *>(eagl::allocator(36, "EAGL::ETTObject new"));
        if (dictionary)
            techniques::object_construct(dictionary, reinterpret_cast<Dword *>(method[2]));
        method[3] = reinterpret_cast<Dword>(dictionary);
    }
    int groups = static_cast<int>(resources::technique_count(reinterpret_cast<Dword *>(method[3])));
    Dword bytes = static_cast<Dword>(groups) * 4 + 4;
    auto *header = static_cast<Dword *>(eagl::allocator(bytes, "Pcode to Use"));
    method[1] = reinterpret_cast<Dword>(header);
    for (Dword i = 0; i < (bytes >> 2); ++i)
        header[i] = 0;
    *header = bytes;
    method[1] = reinterpret_cast<Dword>(header + 1);
    Dword first = 0;
    for (int group = 0; group < groups; ++group) {
        int selected = static_cast<int>(resources::technique_at(
            reinterpret_cast<Dword *>(method[3]), group));
        auto *choices = reinterpret_cast<Dword *>(method[1]);
        if (method[0] && !choices[group] && selected >= 0) {
            choices[group] = reinterpret_cast<Dword>(resources::find_technique(
                selected, reinterpret_cast<Dword *>(method[0])));
            if (!method[6] && method[9]) {
                if (!group) {
                    first = selected;
                    build_buffers(reinterpret_cast<Dword *>(method[9]), selected);
                } else
                    build_variant(reinterpret_cast<Dword *>(method[9]), selected, first);
            }
        }
    }
}

void initialize_variant(Dword *method) {
    auto *source = reinterpret_cast<Dword *>(method[8]);
    method[2] = source[2];
    method[6] = source[6];
    method[5] = source[5];
    method[4] = 0;
    method[1] = 0;
    method[3] = source[3];
    method[9] = source[9];
    method[10] = source[10];
    auto *code = reinterpret_cast<Dword *>(source[0]);
    Dword index = 0;
    for (;;) {
        Dword step = code[index] & 65535;
        if (step) {
            index += step;
            continue;
        }
        if (!(code[index + 1] & 65535))
            break;
        ++index;
    }
    Dword code_bytes = (index + 2) * 4 + 4;
    auto *header = static_cast<Dword *>(eagl::allocator(code_bytes,
        "EAGL::RenderMethod(RenderMethod &parent) allocating PCode block"));
    method[0] = reinterpret_cast<Dword>(header);
    *header = code_bytes;
    method[0] = reinterpret_cast<Dword>(header + 1);
    kernel::memory_copy(header + 1, reinterpret_cast<Dword *>(source[0]), (index + 2) * 4);
    int groups = static_cast<int>(resources::technique_count(reinterpret_cast<Dword *>(method[3])));
    Dword bytes = static_cast<Dword>(groups) * 4 + 4;
    header = static_cast<Dword *>(eagl::allocator(bytes, "Pcode to Use"));
    method[1] = reinterpret_cast<Dword>(header);
    for (Dword i = 0; i < (bytes >> 2); ++i)
        header[i] = 0;
    *header = code_bytes;
    method[1] = reinterpret_cast<Dword>(header + 1);
    for (int group = 0; group < groups; ++group) {
        int selected = static_cast<int>(resources::technique_at(
            reinterpret_cast<Dword *>(method[3]), group));
        auto *choices = reinterpret_cast<Dword *>(method[1]);
        if (method[0] && !choices[group] && selected >= 0) {
            choices[group] = reinterpret_cast<Dword>(resources::find_technique(
                selected, reinterpret_cast<Dword *>(method[0])));
            if (!method[6] && method[9]) {
                if (!group)
                    build_buffers(reinterpret_cast<Dword *>(method[9]), selected);
                else
                    build_variant(reinterpret_cast<Dword *>(method[9]), selected, group);
            }
        }
    }
}
} // namespace nfs::graphics::methods
