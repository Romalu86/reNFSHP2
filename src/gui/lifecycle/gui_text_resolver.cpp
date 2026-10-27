#include "nfs/core/lifecycle/kernel_initializers.hpp"
#include <cstdlib>
#include "nfs/core/errors/assertions.hpp"
#include "gui_input.hpp"
#include "nfs/network/session/network_rules.hpp"
#include "nfs/core/lifecycle/subsystem_runtime.hpp"
#include <cstring>
#include <cwchar>
namespace nfs::network::rules::pending {
extern void *language_data;
extern Dword unicode_language;
} // namespace nfs::network::rules::pending
namespace nfs::gui::input {
using game::runtime::at;
using game::runtime::system;
extern const char *const text_names[2629] = {
#include "gui_text_names.inc"
};

const void *language_text(int id) {
    if (!id || id > 2628)
        return nullptr;
    using namespace network::rules::pending;
    if (!language_data)
        load_language(at<Dword>(system(72), 2312));
    return reinterpret_cast<const void *>(reinterpret_cast<Dword>(language_data) +
                                          at<Dword>(language_data, Dword(id) * 4));
}

static int language_text_length(int id) {
    using namespace network::rules::pending;
    if (!id || id >= 2629)
        return 0;
    const void *text = language_text(id);
    if (!text)
        return 0;
    const void *next = language_text(id + 1);
    if (next) {
        int length = int(reinterpret_cast<Dword>(next) - reinterpret_cast<Dword>(text));
        if (unicode_language)
            length /= 2;
        return length - 1;
    }
    return unicode_language ? int(std::wcslen(static_cast<const wchar_t *>(text)))
                            : int(std::strlen(static_cast<const char *>(text)));
}
extern const engine::Method resolver_methods[7], resolver_base_methods[7];
struct Resolver {
    const engine::Method *methods;
};

static Resolver *construct_resolver(Resolver *resolver) {
    resolver->methods = resolver_base_methods;
    resolver->methods = resolver_methods;
    return resolver;
}

static void destroy_resolver(Resolver *resolver) {
    resolver->methods = resolver_base_methods;
}

static Resolver *__fastcall delete_resolver(Resolver *resolver, void *, Dword flags) {
    destroy_resolver(resolver);
    if (flags & 1)
        application_release(resolver);
    return resolver;
}

static Resolver *__fastcall delete_resolver_base(Resolver *resolver, void *, Dword flags) {
    resolver->methods = resolver_base_methods;
    if (flags & 1)
        application_release(resolver);
    return resolver;
}

static const void *__fastcall resolve(Resolver *, void *, int id) {
    return id > 0 && id < 2629 ? language_text(id) : nullptr;
}

static Dword __fastcall is_wide(Resolver *, void *, Dword) {
    return network::rules::pending::unicode_language;
}
// Prefix lookup is case sensitive.
const char *const reference_prefixes[3] = {"kText", "kTxt", "kMenu"};

static Dword __fastcall reference_for_name(Resolver *, void *, const char *name) {
    if (!name)
        return 0;
    bool prefixed = false;
    for (const char *prefix : reference_prefixes)
        if (std::strstr(name, prefix) == name) {
            prefixed = true;
            break;
        }
    if (prefixed)
        for (Dword id = 1; id < 2629; ++id)
            if (!std::strcmp(name, text_names[id]))
                return id;
    return 0;
}

static int __fastcall resolved_length(Resolver *, void *, int id) {
    if (id <= 0 || id >= 2629)
        return 0;
    const int length = language_text_length(id);
    return length < 0 ? 0 : length;
}

static const char *__fastcall reference_name(Resolver *, void *, int id) {
    return id > 0 && id < 2629 ? text_names[id] : nullptr;
}

static void __fastcall visit_references(Resolver *, void *, Dword(__cdecl *visitor)(Dword, Dword),
                                        Dword context) {
    for (Dword id = 1; id < 2629; ++id)
        if (!visitor(id, context))
            return;
}
#define METHOD(name) reinterpret_cast<engine::Method>(name)
const engine::Method resolver_methods[7] = {METHOD(delete_resolver), METHOD(resolve),
                                            METHOD(is_wide),         METHOD(reference_for_name),
                                            METHOD(resolved_length), METHOD(reference_name),
                                            METHOD(visit_references)};
const engine::Method resolver_base_methods[7] = {
    METHOD(delete_resolver_base), METHOD(::nfs::assertions::pure_virtual_call), METHOD(::nfs::assertions::pure_virtual_call), METHOD(::nfs::assertions::pure_virtual_call),
    METHOD(::nfs::assertions::pure_virtual_call),         METHOD(::nfs::assertions::pure_virtual_call), METHOD(::nfs::assertions::pure_virtual_call)};
#undef METHOD
static Resolver resolver_value{};

static void finalize_resolver() {
    destroy_resolver(&resolver_value);
}

static void initialize_resolver() {
    construct_resolver(&resolver_value);
    std::atexit(finalize_resolver);
}
NFS_KERNEL_INITIALIZER(startup_initialize_resolver, ".CRT$XCU_NFS_0069013C",
                       initialize_resolver);

void bind_text_resolver(Object *resolver) {
    string_resolver = resolver;
}
Object *text_resolver() {
    return reinterpret_cast<Object *>(&resolver_value);
}
} // namespace nfs::gui::input
