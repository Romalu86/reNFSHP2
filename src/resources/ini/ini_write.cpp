#include "nfs/resources/ini/ini.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstdio>
namespace nfs::ini {
namespace {
const gui::String *name_string(gui::PoolString *name) {
    return name ? &name->string : &lifecycle::construction::unresolved::shared_string;
}
Section *find_reader_section(Document *document, const char *name) {
    gui::Container &sections = document->sections;
    for (int index = 0; index < static_cast<int>(sections.count); ++index) {
        gui::Element *element = sections.elements[index];
        auto *section = element ? static_cast<Section *>(element->value) : nullptr;
        if (section && gui::string_equal_folded(name_string(section->name), name))
            return section;
    }
    return nullptr;
}
Key *find_reader_key(Document *document, const char *section_name, const char *key_name) {
    Section *section = find_reader_section(document, section_name);
    if (!section)
        return nullptr;
    for (int index = 0; index < static_cast<int>(section->keys.count); ++index) {
        gui::Element *element = section->keys.elements[index];
        auto *key = element ? static_cast<Key *>(element->value) : nullptr;
        if (key && gui::string_equal_folded(name_string(key->name), key_name))
            return key;
    }
    return nullptr;
}
// The four saved setters keep their original Document receiver across read_only.
Key *find_or_add_key(Document *document, const char *section_name, const char *key_name) {
    Key *key = find_reader_key(document, section_name, key_name);
    if (key)
        return key;
    Section *section = find_reader_section(document, section_name);
    if (!section)
        section = add_section(document, section_name, 0);
    return set(section, key_name, nullptr, 1);
}
} // namespace

std::uint8_t set_string(Reader *reader, const char *section_name, const char *key_name, const char *text) {
    CachedDocument *document = reader->document;
    if (!document || read_only(&document->document))
        return 0;
    Key *key = find_or_add_key(&document->document, section_name, key_name);
    if (!key)
        return 0;
    gui::string_assign(&key->value, text);
    reader->modified = 1;
    return 1;
}
// Both entries format the raw word with %d.
static std::uint8_t write_number(Reader *reader, const char *section_name, const char *key_name, Dword value) {
    CachedDocument *document = reader->document;
    if (!document)
        return 0;
    char text[256];
    std::sprintf(text, "%d", static_cast<int>(value));
    if (read_only(&document->document))
        return 0;
    Key *key = find_or_add_key(&document->document, section_name, key_name);
    if (!key)
        return 0;
    gui::string_assign(&key->value, text);
    reader->modified = 1;
    return 1;
}
std::uint8_t set_int(Reader *reader, const char *section, const char *key, int value) {
    return write_number(reader, section, key, static_cast<Dword>(value));
}
std::uint8_t set_word(Reader *reader, const char *section, const char *key, Dword value) {
    return write_number(reader, section, key, value);
}
// Preserve separate precision-format and value buffers.
std::uint8_t set_float(Reader *reader, const char *section_name, const char *key_name, float value,
               int precision) {
    CachedDocument *document = reader->document;
    if (!document)
        return 0;
    char format[256], text[256];
    std::sprintf(format, "%%0.%df", precision);
    std::sprintf(text, format, static_cast<double>(value));
    if (read_only(&document->document))
        return 0;
    Key *key = find_or_add_key(&document->document, section_name, key_name);
    if (!key)
        return 0;
    gui::string_assign(&key->value, text);
    reader->modified = 1;
    return 1;
}
} // namespace nfs::ini
