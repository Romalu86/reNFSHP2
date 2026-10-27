#include "nfs/network/session/network_world.hpp"
#include "nfs/ai/lifecycle/ai_lifecycle.hpp"
namespace nfs::network::world {
Dword *&vehicle_registry=*reinterpret_cast<Dword**>(game::runtime::ai::lifecycle::storage+332);
namespace {
Dword __fastcall iterator_first(Iterator *object, void *) {
    return first(object);
}
Dword __fastcall iterator_next(Iterator *object, void *) {
    return next(object);
}
const Dword iterator_methods[] = {reinterpret_cast<Dword>(iterator_first),
                                 reinterpret_cast<Dword>(iterator_next)};
} // namespace
// Exact two-method table at .
Iterator *construct_iterator(Iterator *object, Dword *array) {
    object->array = array;
    object->index = 0;
    object->methods = iterator_methods;
    return object;
}

Dword first(Iterator *object) {
    object->index = 0;
    return object->array[0] ? object->array[1] : 0;
}

Dword next(Iterator *object) {
    const Dword previous = object->index++;
    return object->array[0] <= object->index ? 0 : object->array[previous + 2];
}

Dword vehicle_slot(void *object) {
    void *state = field<void *>(object, 368);
    return state ? field<Dword>(state, 1772) : 4294967295u;
}
} // namespace nfs::network::world
