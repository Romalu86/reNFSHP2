#pragma once
#include "nfs/core/types/kernel.hpp"
#include <cstdarg>
namespace nfs::assertions {
using kernel::Dword;
extern const char empty_text[1];
extern const char *source, *description;
extern Dword line;
extern unsigned char abort_only;
void prepare();
void set_abort_only(unsigned char);
unsigned char report_v(const char *, std::va_list);
unsigned char report(const char *, ...);
Dword report_details(const char *, Dword, const char *, const char *);
void pure_virtual_call();
} // namespace nfs::assertions
