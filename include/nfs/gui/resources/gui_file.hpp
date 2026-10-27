#pragma once
#include "nfs/gui/lifecycle/gui.hpp"
namespace nfs::gui {
struct File {const Destroy* methods;Dword file;int offset,priority;};
static_assert(sizeof(File)==16);
void file_construct(File*,const char*,std::uint8_t,int);void file_close(File*);
void file_destruct(File*);
File* file_write(File*,const String*);File* file_write(File*,const char*);
}
