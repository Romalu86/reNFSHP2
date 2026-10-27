#pragma once
#include "../../ai/driving/ai_driver_runtime.hpp"
namespace nfs::game::runtime::ai::cars {
using BindingReader = int(__cdecl *)(Dword, Byte *);
extern const BindingReader binding_readers[5];
void read_cop_action_bindings(void *, Dword, Dword *);
} // namespace nfs::game::runtime::ai::cars
