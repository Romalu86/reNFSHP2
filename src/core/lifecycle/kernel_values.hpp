#pragma once
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::game::runtime::values {
using kernel::Dword;
struct TaggedValue;
void clear_value(TaggedValue *);
void reset_value(TaggedValue *);
void destroy_value(TaggedValue *);
bool equal_values(const TaggedValue *, const TaggedValue *);
extern const gui::Destroy tagged_methods[], vector_methods[];
struct TaggedValue {
    const gui::Destroy *methods;
    Dword type, payload[4];
    TaggedValue() {
        methods = gui::base_methods;
        methods = tagged_methods;
        clear_value(this);
    }
    ~TaggedValue() {
        destroy_value(this);
    }
};
static_assert(sizeof(TaggedValue) == 24);
struct VectorValue {
    const gui::Destroy *methods;
    float x, y, z;
};
static_assert(sizeof(VectorValue) == 16);
void destroy_vector(VectorValue *);
VectorValue *sum_vector_value(VectorValue *, const VectorValue *, const float *);
VectorValue *assign_vector_value(VectorValue *, const float *);
float *difference_triplets(float *, const float *, const float *);
} // namespace nfs::game::runtime::values
