#include "movie_runtime.hpp"
namespace nfs::movies {
Memory memory{};
Byte stop_requested{};

void initialize_memory(Memory *owner) {
    owner->allocate = kernel::memory_allocate_aligned;
    owner->release = kernel::memory_free;
    owner->pool = 0;
}
} // namespace nfs::movies
