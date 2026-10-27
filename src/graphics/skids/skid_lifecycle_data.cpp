#include "nfs/graphics/skids/skid_lifecycle.hpp"
namespace nfs::game::runtime::skids::lifecycle {
alignas(4) Byte records[1500 * 84]{};
Byte *free_head = records;
const Dword identity_matrix[16] = {1065353216, 0, 0,          0, 0, 1065353216, 0, 0,
                                  0,          0, 1065353216, 0, 0, 0,          0, 1065353216};

void initialize_free_list() {
    for (unsigned index = 0; index < 1499; ++index)
        at<void *>(records, index * 84) = records + (index + 1) * 84;
    at<Dword>(records, 1499 * 84) = 0;
    free_head = records;
}

void *allocate_skid() {
    Byte *result = free_head;
    if (!result)
        return nullptr;
    free_head = at<Byte *>(result, 0);
    kernel::memory_zero(result, 84);
    return result;
}

void release_skid(void *node) {
    if (!node)
        return;
    kernel::memory_zero(node, 84);
    at<Byte *>(node, 0) = free_head;
    free_head = static_cast<Byte *>(node);
}

void initialize(Dword *owner) {
    owner[1] = owner[2] = 0;
    initialize_free_list();
}
} // namespace nfs::game::runtime::skids::lifecycle
