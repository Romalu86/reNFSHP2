#pragma once
#include "nfs/network/session/network_virtual.hpp"
namespace nfs::lifecycle::construction::network::queue {
using kernel::Dword;
Dword count(Dword*);Dword selected_count(Dword*);Dword consume_changed(Dword*);Dword mark_changed(Dword*);
void clear(Dword*);
void visit(Dword*,Dword,Dword,Dword*,Dword,Dword*);Dword append(Dword*,Dword,Dword);
Dword set_selected(Dword*,Dword,Dword,Dword*);Dword replace(Dword*,Dword,Dword,Dword*);Dword erase(Dword*,Dword,Dword*);
Dword copy_matching(Dword*,Dword,Dword,Dword*);Dword copy_selected(Dword*,Dword);Dword copy_nth(Dword*,Dword,int,Dword,Dword,Dword*,Dword,Dword*);Dword take(Dword*,Dword,Dword);
void unlink(Dword*,Dword*);Dword* find(Dword*,Dword,Dword*);void select(Dword*,Dword*,Dword);
}
