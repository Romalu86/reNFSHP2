#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::network::digest {
using kernel::Dword;
struct State {
    Dword hash[4], bits_low, bits_high;
    std::uint8_t block[64];
};
static_assert(sizeof(State) == 88);
void transform(State *, const void *);
void update(State *, const void *, Dword);
void hexadecimal(const void *, Dword, char *);
} // namespace nfs::network::digest
