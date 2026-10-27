#pragma once
#include "camera_director_runtime.hpp"
#include <cstdio>
namespace nfs::game::world::camera {
template <class Read> void read_camera_configuration(Read read) {
    char path[260];
    std::sprintf(path, "%sCameras.ini",
                 reinterpret_cast<const char *>(offset(subsystem(4), 2084)));
    if (files::file::exists(path)) {
        ini::Reader reader;
        ini::reader_construct(&reader, path, 0);
        try {
            read(&reader);
        } catch (...) {
            ini::reader_clear(&reader);
            throw;
        }
        ini::reader_clear(&reader);
    }
}
template <class Construct>
void *allocate_behavior(Dword size, const char *name, Construct construct) {
    void *memory = lifecycle::allocate(size, name, lifecycle::level_pool);
    return memory ? construct(memory) : nullptr;
}
inline void *allocate_controller(void *director, Dword identifier, bool copy_up = true) {
    void *controller = allocate_behavior(108, "camera controller", construct_controller);
    at<Dword>(controller, 76) = identifier;
    at<Dword>(controller, 104) = identifier;
    at<void *>(controller, 4) = director;
    if (copy_up)
        for (unsigned axis = 0; axis < 3; ++axis)
            at<float>(controller, 64 + axis * 4) = up_direction[axis];
    return controller;
}
} // namespace nfs::game::world::camera
