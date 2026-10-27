#pragma once

namespace nfs::kernel::initializers {
using Function = void(__cdecl *)();
}

// CRT section names determine initializer order. Each handler constructs its state and registers
// its exit callbacks.
#define NFS_KERNEL_INITIALIZER(name, segment_name, function)                              \
    __pragma(section(segment_name, long, read))                                          \
    __declspec(allocate(segment_name))                                                  \
    extern ::nfs::kernel::initializers::Function const name = function
