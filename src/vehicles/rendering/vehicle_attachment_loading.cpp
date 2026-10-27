#include "vehicle_renderable_resources.hpp"
#include "nfs/core/lifecycle/system_runtime.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include "nfs/graphics/math/eagl_math.hpp"
#include "../../math/precision/extended_float.hpp"
#include <cstdio>
#include <cstring>
namespace nfs::game::world::vehicle_render {
Dword *animation_pose_scratch() {
    return reinterpret_cast<Dword *>(lifecycle::runtime::animation_cursors[1]);
}
Dword *animation_matrix_scratch() {
    return reinterpret_cast<Dword *>(lifecycle::runtime::animation_cursors[2]);
}
Dword *animation_world_scratch() {
    return reinterpret_cast<Dword *>(lifecycle::runtime::animation_cursors[3]);
}
char attachment_name_scratch[260]{};
                                     // open.
SkeletonMultiply skeleton_multiply = pending::multiply_skeleton_matrix;
// The table contains this single deleting destructor.
void *__fastcall delete_skeleton(void *owner, void *, Dword flags) {
    at<const Dword *>(owner, 0) = skeleton_methods;
    if (at<Byte>(owner, 112)) {
        if (auto *loader = at<eagl::Loader *>(owner, 16)) {
            eagl::loader_destroy(loader);
            eagl::deallocator(loader, 28);
            at<Dword>(owner, 16) = 0;
            if (void *image = at<void *>(owner, 20)) {
                kernel::memory_free(image);
                at<Dword>(owner, 20) = 0;
            }
        }
    }
    if (flags & 1)
        gui::application_release(owner);
    return owner;
}
const Dword skeleton_methods[1] = {
    reinterpret_cast<Dword>(delete_skeleton)};

void *construct_skeleton(void *owner) {
    at<const Dword *>(owner, 0) = skeleton_methods;
    at<Byte>(owner, 112) = 0;
    for (unsigned field = 16; field <= 36; field += 4)
        at<Dword>(owner, field) = 0;
    lifecycle::construction::identity_matrix(reinterpret_cast<Dword *>(offset(owner, 48)));
    return owner;
}

void bind_skeleton(void *owner, eagl::Loader *loader) {
    at<eagl::Loader *>(owner, 16) = loader;
    int cursor = 0;
    eagl::next_address(loader, "Skeleton", &cursor, &at<Dword>(owner, 24));
    at<Byte>(owner, 112) = 0;
}

Byte skeleton_has_node(void *owner, const char *name) {
    Dword address;
    return eagl::address(at<eagl::Loader *>(owner, 16), "Bone", name, &address);
}

void *reserve_skeleton_node(void *pool) {
    for (unsigned index = 0; index < 256; ++index)
        if (!at<Byte>(pool, 24592 + index)) {
            at<Byte>(pool, 24592 + index) = 1;
            return offset(pool, 16 + index * 96);
        }
    return nullptr;
}

void insert_skeleton_node(void *owner, Dword index, void *node) {
    at<Dword>(node, 80) = index;
    at<Dword>(node, 84) = at<Dword>(owner, 28);
    at<void *>(owner, 28) = node;
}

void create_attachment_node(void *owner, Dword index) {
    const char *name = attachment_names[index];
    at<Dword>(owner, 48 + index * 8) = 0;
    at<Dword>(owner, 52 + index * 8) = index;
    void *node = reserve_skeleton_node(subsystem(44));
    at<void *>(owner, 48 + index * 8) = node;
    void *skeleton = at<void *>(owner, 408);
    insert_skeleton_node(skeleton, pending::skeleton_node_index(skeleton, name), node);
}

Dword attachment_index(const char *name) {
    std::sprintf(attachment_name_scratch, "Root.%s", name);
    for (unsigned index = 0; index < 45; ++index)
        if (!std::strcmp(attachment_name_scratch, attachment_names[index]))
            return index;
    return Dword(-1);
}

void pending::load_attachments(void *owner, eagl::Loader *loader) {
    if (!graphics::device)
        return;
    void *skeleton = lifecycle::allocate(128, "Vehicle Skeleton", lifecycle::cars_pool);
    if (skeleton) {
        try {
            construct_skeleton(skeleton);
        } catch (...) {
            gui::application_release(skeleton);
            throw;
        }
    }
    at<void *>(owner, 408) = skeleton;
    bind_skeleton(skeleton, loader);
    for (unsigned index = 0; index < 45; ++index)
        at<Byte>(owner, index) = skeleton_has_node(skeleton, attachment_names[index]);
    constexpr Dword initial_nodes[] = {40, 19, 20, 37, 10, 9, 8, 7};
    for (Dword index : initial_nodes)
        if (at<Byte>(owner, index))
            create_attachment_node(owner, index);
}

void restore_skeleton_pose(void *skeleton, Dword *output, const Dword *mask) {
    for (int index = 0; index < at<int>(skeleton, 8); ++index) {
        if (mask && !(mask[Dword(index) >> 5] & (Dword(1) << (Dword(index) & 31))))
            continue;
        auto *bone = reinterpret_cast<Dword *>(offset(skeleton, 16 + Dword(index) * 112));
        Dword *pose = output + Dword(index) * 12;
        pose[0] = bone[0];
        pose[1] = bone[1];
        pose[2] = bone[2];
        pose[3] = at<Dword *>(skeleton, 12) ? at<Dword *>(skeleton, 12)[index] : 1065353216;
        for (unsigned channel = 0; channel < 7; ++channel)
            pose[4 + channel] = bone[4 + channel];
        pose[11] = 1065353216;
    }
}

void pending::initialize_attachment_root(void *owner) {
    restore_skeleton_pose(at<void *>(at<void *>(owner, 408), 24), animation_pose_scratch(),
                          nullptr);
}
// Word copies retain in-place transpose order.
void transpose_skeleton_matrix(const Dword *input, Dword *output) {
    output[0] = input[0];
    Dword saved = input[1];
    output[1] = input[4];
    output[4] = saved;
    saved = input[8];
    output[8] = input[2];
    output[2] = saved;
    saved = input[3];
    output[3] = input[12];
    output[12] = saved;
    output[5] = input[5];
    saved = input[6];
    output[6] = input[9];
    output[9] = saved;
    saved = input[13];
    output[13] = input[7];
    output[7] = saved;
    output[10] = input[10];
    saved = input[11];
    output[11] = input[14];
    output[14] = saved;
    output[15] = input[15];
}

void bind_skeleton_matrix(const Dword *input, Dword *output) {
    transpose_skeleton_matrix(input, output);
}
namespace {
bool selected(const Dword *mask, Dword index) {
    return !mask || (mask[index >> 5] & (Dword(1) << (index & 31)));
}
void pose_matrix(const float *pose, float *matrix) {
    const auto format = kernel::numeric::current_format();
    auto X = [&](float value) { return kernel::numeric::Extended(value, format); };
    const float scale_x = pose[0], scale_y = pose[1], scale_z = pose[2], blend = pose[3];
    const float x = pose[4], y = pose[5], z = pose[6], w = pose[7];
    const float tx = pose[8], ty = pose[9], tz = pose[10];
    float twice_x = float(X(x) + x), twice_y = float(X(y) + y), twice_z = float(X(z) + z);
    float xw = float(X(twice_x) * w), yw = float(X(twice_y) * w), zw = float(X(twice_z) * w);
    float xx = float(X(twice_x) * x), xy = float(X(twice_y) * x), xz = float(X(twice_z) * x);
    float yy = float(X(twice_y) * y), yz = float(X(twice_z) * y);
    auto zz = X(twice_z) * z; // The final product remains on the saved x87 stack.
    matrix[0] = float((X(1) - (X(yy) + zz)) * scale_x);
    matrix[4] = float((X(xy) - zw) * scale_y);
    matrix[8] = float((X(xz) + yw) * scale_z);
    matrix[1] = float((X(xy) + zw) * scale_x);
    matrix[5] = float((X(1) - (zz + xx)) * scale_y);
    matrix[3] = matrix[7] = matrix[11] = 0;
    matrix[15] = 1;
    matrix[9] = float((X(yz) - xw) * scale_z);
    matrix[2] = float((X(xz) - yw) * scale_x);
    matrix[6] = float((X(yz) + xw) * scale_y);
    matrix[10] = float((X(1) - (X(yy) + xx)) * scale_z);
    matrix[12] = tx;
    matrix[13] = ty;
    matrix[14] = tz;
    matrix[0] = float(X(blend) * matrix[0]);
    matrix[4] = float(X(blend) * matrix[4]);
    matrix[8] = float(X(blend) * matrix[8]);
}
} // namespace

void convert_skeleton_pose(void *skeleton, const float *input, float *output, const Dword *mask) {
    for (int index = 0; index < at<int>(skeleton, 8); ++index)
        if (selected(mask, Dword(index)))
            pose_matrix(input + Dword(index) * 12, output + Dword(index) * 16);
}

void bind_skeleton_matrices(void *skeleton, const Dword *input, Dword *output, const Dword *mask) {
    for (int index = 0; index < at<int>(skeleton, 8); ++index) {
        if (!selected(mask, Dword(index)))
            continue;
        int parent = at<int>(skeleton, 28 + Dword(index) * 112);
        Dword *matrix = output + Dword(index) * 16;
        const Dword *local = input + Dword(index) * 16;
        if (parent < 0) {
            for (unsigned word = 0; word < 16; ++word)
                matrix[word] = local[word];
        } else
            skeleton_multiply(reinterpret_cast<float *>(matrix),
                              reinterpret_cast<const float *>(output + Dword(parent) * 16),
                              reinterpret_cast<const float *>(local));
    }
}

void accumulate_skeleton_matrices(void *skeleton, const float *input, float *output,
                                  const Dword *mask) {
    for (int index = 0; index < at<int>(skeleton, 8); ++index) {
        if (!selected(mask, Dword(index)))
            continue;
        float *matrix = output + Dword(index) * 16;
        pose_matrix(input + Dword(index) * 12, matrix);
        int parent = at<int>(skeleton, 28 + Dword(index) * 112);
        if (parent >= 0)
            skeleton_multiply(matrix, output + Dword(parent) * 16, matrix);
    }
}

void apply_skeleton_bind_pose(void *skeleton, const float *input, float *output, const Dword *mask) {
    for (int index = 0; index < at<int>(skeleton, 8); ++index) {
        if (!selected(mask, Dword(index)))
            continue;
        float *matrix = output + Dword(index) * 16;
        skeleton_multiply(matrix, input + Dword(index) * 16,
                          reinterpret_cast<float *>(offset(skeleton, 64 + Dword(index) * 112)));
        bind_skeleton_matrix(reinterpret_cast<Dword *>(matrix), reinterpret_cast<Dword *>(matrix));
    }
}
// Masked path still accumulates every input bone first.
void prepare_skeleton_bind_pose(void *skeleton, const float *input, float *output,
                                const Dword *mask) {
    accumulate_skeleton_matrices(skeleton, input, output, nullptr);
    apply_skeleton_bind_pose(skeleton, output, output, mask);
}

void evaluate_skeleton(void *owner, Dword *pose) {
    void *skeleton = at<void *>(owner, 24);
    Dword *matrices = animation_matrix_scratch();
    if (!at<Dword>(owner, 28) && !at<Dword>(owner, 36) && !at<Dword>(owner, 32)) {
        prepare_skeleton_bind_pose(skeleton, reinterpret_cast<float *>(pose),
                                   reinterpret_cast<float *>(matrices), nullptr);
        return;
    }
    convert_skeleton_pose(skeleton, reinterpret_cast<float *>(pose),
                          reinterpret_cast<float *>(matrices), nullptr);
    Dword *world = animation_world_scratch();
    float root[16];
    graphics::math::multiply(reinterpret_cast<float *>(offset(owner, 48)),
                             reinterpret_cast<float *>(matrices), root);
    std::memcpy(matrices, root, 64);
    bind_skeleton_matrices(skeleton, matrices, world, nullptr);
    world = animation_world_scratch();
    for (void *node = at<void *>(owner, 28); node; node = at<void *>(node, 84))
        std::memcpy(offset(node, 16), world + at<Dword>(node, 80) * 16, 64);
    apply_skeleton_bind_pose(skeleton, reinterpret_cast<float *>(world),
                             reinterpret_cast<float *>(animation_matrix_scratch()), nullptr);
}

void pending::evaluate_attachments(void *owner) {
    evaluate_skeleton(at<void *>(owner, 408), animation_pose_scratch());
}
} // namespace nfs::game::world::vehicle_render
