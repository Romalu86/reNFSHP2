#include "nfs/core/errors/assertions.hpp"
#include "movie_runtime.hpp"
#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/gui/lifecycle/gui.hpp"
#include <cstring>
#include <new>
#include <stdexcept>
namespace nfs::movies {
namespace {
Dword *create_movie_texture() {
    auto *texture = static_cast<Dword *>(eagl::allocator(48, "EAGL::TAR new"));
    if (!texture)
        throw std::bad_alloc();
    Dword *shared = nullptr;
    try {
        shared = static_cast<Dword *>(eagl::allocator(56, "EAGLInternal::SharedData new"));
        if (!shared)
            throw std::bad_alloc();
        // Movie-only compatibility: no uninitialized sampler/reserved owner bits
        // may reach the driver on any successive movie. Ordinary TAR is unchanged.
        std::memset(texture, 0, 48);
        std::memset(shared, 0, 56);
        graphics::tar::base_construct(texture);
        graphics::tar::extension_construct(texture + 10, texture);
        graphics::tar::shared_construct(shared);
        shared[12] |= 2;
        ++shared[0];
        texture[9] = reinterpret_cast<Dword>(shared);
        texture[1] = texture[2] = texture[3] = texture[4] = 3;
        register_movie_texture(texture);
        return texture;
    } catch (...) {
        if (shared)
            eagl::deallocator(shared, 56);
        eagl::deallocator(texture, 48);
        throw;
    }
}
}
const Dword renderer_methods[6] = {
    reinterpret_cast<Dword>(delete_renderer), reinterpret_cast<Dword>(position),
    reinterpret_cast<Dword>(scale),           reinterpret_cast<Dword>(draw_frame),
    reinterpret_cast<Dword>(redraw),          reinterpret_cast<Dword>(release_presented_frame)};
const Dword renderer_base_methods[6] = {
    reinterpret_cast<Dword>(delete_renderer_base),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(::nfs::assertions::pure_virtual_call),
    reinterpret_cast<Dword>(release_presented_frame)};
// The outer movie caller supplies one additional ignored stack word.
Renderer *create_renderer(Delegate *delegate, void *frame, graphics::Device *device, Dword *context,
                          Dword *immediate, int format) {
    auto *renderer = static_cast<Renderer *>(memory.allocate("", 24, 0, 0, memory.pool));
    if (!renderer)
        return nullptr;
    try {
        return construct_renderer(renderer, delegate, frame, device, context, immediate, format);
    } catch (...) {
        memory.release(renderer);
        throw;
    }
}

Renderer *construct_renderer(Renderer *renderer, Delegate *delegate, void *frame,
                             graphics::Device *device, Dword *context, Dword *immediate, int format) {
    renderer->delegate = delegate;
    renderer->methods = renderer_methods;
    renderer->image = nullptr;
    renderer->texture = nullptr;
    renderer->current = 0;
    renderer->count = 1;
    auto *image = static_cast<Image *>(memory.allocate("", 60, 0, 0, memory.pool));
    if (!image)
        throw std::bad_alloc();
    image->shape = nullptr;
    try {
        construct_image(image, frame, device, context, immediate, format);
        if (!image->shape)
            throw std::bad_alloc();
        renderer->image = image;
        renderer->texture = create_movie_texture();
    } catch (...) {
        if (image->shape)
            destroy_image(image);
        memory.release(image);
        renderer->image = nullptr;
        throw;
    }
    return renderer;
}

void destroy_renderer(Renderer *renderer) {
    Dword *texture = renderer->texture;
    renderer->methods = renderer_methods;
    if (texture)
        destroy_movie_texture(texture);
    renderer->texture = nullptr;
    if (renderer->image)
        invoke<void *>(renderer->image, 0, 1u);
    renderer->image = nullptr;
    renderer->methods = renderer_base_methods;
}

void *__fastcall delete_renderer(Renderer *renderer, void *, unsigned flags) {
    destroy_renderer(renderer);
    if (flags & 1)
        memory.release(renderer);
    return renderer;
}

void *__fastcall delete_renderer_base(Renderer *renderer, void *, unsigned flags) {
    renderer->methods = renderer_base_methods;
    if (flags & 1)
        gui::application_release(renderer);
    return renderer;
}

void __fastcall position(Renderer *renderer, void *, float x, float y, float z) {
    renderer->image->x = x;
    renderer->image->y = y;
    renderer->image->z = z;
}

void __fastcall scale(Renderer *renderer, void *, float x, float y) {
    renderer->image->scale_x = x;
    renderer->image->scale_y = y;
}
// Authorized movie transport split: conversion/upload/retirement precede BeginScene.
void prepare_frame(Renderer *renderer, void *frame) {
    renderer->current = int(Dword(renderer->current) + 1);
    if (renderer->current >= 1)
        renderer->current = 0;
    // The saved array selection is unchecked, even though its normal index is zero.
    Image *image = field<Image *>(renderer, 16 + Dword(renderer->current) * 4);
    copy_frame(image, frame, renderer->texture);
    retire_frame(renderer->delegate, frame);
}
void prepare_presented_frame(Renderer *renderer) {
    if (movie_texture_needs_upload(renderer->texture))
        upload_frame(field<Image *>(renderer, 16 + Dword(renderer->current) * 4),
                     renderer->texture);
}

void __fastcall draw_frame(Renderer *renderer, void *, void *frame) {
    prepare_frame(renderer, frame);
    draw_image(field<Image *>(renderer, 16 + Dword(renderer->current) * 4), renderer->texture);
}

void __fastcall redraw(Renderer *renderer, void *) {
    draw_image(field<Image *>(renderer, 16 + Dword(renderer->current) * 4), renderer->texture);
}
 
void __fastcall release_presented_frame(Renderer *, void *) {
}
} // namespace nfs::movies
