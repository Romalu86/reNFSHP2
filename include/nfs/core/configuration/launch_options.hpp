#pragma once
#include "nfs/core/types/kernel.hpp"
namespace nfs::launch {
using kernel::Dword;
struct Options {
    Dword flags[77];
    char values[77][24];
};
static_assert(sizeof(Options) == 2156 && offsetof(Options, values) == 308);
extern Options options;
extern unsigned char network_requested;
struct Specification {
    int flag;
    const char *name;
};
extern const Specification specifications[78];
template <class T> T &field(unsigned va) {
    return *reinterpret_cast<T *>(reinterpret_cast<unsigned char *>(&options) + va - 8627648);
}
Options *construct(Options *);
const char *option_name(int);
unsigned char collect_quoted_value(int, const char *const *, int *, char *);
Dword parse(Options *, int, const char *const *);
Dword parse_file(Options *);
namespace settings {
extern int remaining;
extern char *cursor, *allocation;
Dword open(const char *);
Dword next(char *, char *, int *);
void close();
} // namespace settings
} // namespace nfs::launch
