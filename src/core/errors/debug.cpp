#include "nfs/core/types/kernel.hpp"
#include <intrin.h>
namespace nfs::kernel {
Dword debug_break_enabled=0;
// Compiled source only; trap branches are not tested.
void debug_break(){if(!debug_break_enabled)return;if(IsDebuggerPresent()){__debugbreak();return;}DebugBreak();}
}
