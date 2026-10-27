#include "nfs/ai/lifecycle/ai_dependencies.hpp"
#include <cstring>
namespace nfs::game::runtime::ai::dependencies {
#include "ai_dependency_names.inc"

int parse_skin_variant(const char *name) {
    if (name)
        for (const auto *entry = skin_names; entry->variant != -1; ++entry)
            if (std::strcmp(name, entry->name) == 0)
                return entry->variant;
    return 0;
}

int validate_skin_variant(int variant) {
    for (const auto *entry = skin_names; entry->name; ++entry)
        if (entry->variant == variant)
            return entry->variant;
    return 0;
}

const char *car_model_name(Dword model) {
    for (const auto *entry = car_names; entry->model != -1; ++entry)
        if (Dword(entry->model) == model)
            return entry->name;
    return "unknown";
}

int parse_car_model(const char *name) {
    for (const auto *entry = car_names; entry->model != -1; ++entry)
        if (std::strcmp(entry->name, name) == 0)
            return entry->model;
    return -1;
}

Dword validate_car_model(int model) {
    for (const auto *entry = car_names; entry->model != -1; ++entry)
        if (entry->model == model)
            return Dword(entry->model);
    return 4294967295u;
}
} // namespace nfs::game::runtime::ai::dependencies
