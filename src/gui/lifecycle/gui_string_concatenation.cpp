#include "nfs/gui/resources/gui_format.hpp"
namespace nfs::gui {

String *append_ascii_value(String *out, const String *source, const char *tail) {
    String temporary;
    string_construct(&temporary);
    try {
        string_copy(&temporary, source);
        string_append_ascii(&temporary, tail);
        string_construct_copy(out, &temporary);
    } catch (...) {
        string_destruct(&temporary);
        throw;
    }
    string_destruct(&temporary);
    return out;
}

String *prepend_ascii_value(String *out, const char *prefix, const String *source) {
    String temporary;
    string_construct(&temporary);
    try {
        string_assign(&temporary, prefix);
        string_append(&temporary, source);
        string_construct_copy(out, &temporary);
    } catch (...) {
        string_destruct(&temporary);
        throw;
    }
    string_destruct(&temporary);
    return out;
}

String *concatenate(String *out, const String *first, const String *second) {
    String temporary;
    string_construct(&temporary);
    try {
        string_copy(&temporary, first);
        string_append(&temporary, second);
        string_construct_copy(out, &temporary);
    } catch (...) {
        string_destruct(&temporary);
        throw;
    }
    string_destruct(&temporary);
    return out;
}
} // namespace nfs::gui
