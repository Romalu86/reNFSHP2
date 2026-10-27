#pragma once
#include "nfs/gui/engine/gui_engine.hpp"
#include "nfs/resources/ini/ini.hpp"
namespace nfs::gui::engine::pending {
struct ConfigurationDocument {
    ini::Document document;
    Dword name_hash, references;
};
static_assert(sizeof(ConfigurationDocument) == 40);
ConfigurationDocument *construct_configuration_document(ConfigurationDocument *, const String *);
Dword configuration_name_hash(const String *);
Dword *hash_name(Dword *, const char *);
} // namespace nfs::gui::engine::pending
