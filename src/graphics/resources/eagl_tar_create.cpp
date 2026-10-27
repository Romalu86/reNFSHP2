#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/graphics/textures/surface_conversion.hpp"
namespace nfs::graphics::tar {
Dword palette_sequence=0;
static Dword* fallback(){return reinterpret_cast<Dword*>(fallback_image+reinterpret_cast<Dword*>(fallback_image)[5]);}
static void palette_entries(Dword* object,const std::uint8_t* source){std::uint8_t entries[1024];for(int i=0;i<256;++i){entries[i*4]=source[i*4+2];entries[i*4+1]=source[i*4+1];entries[i*4+2]=source[i*4];entries[i*4+3]=source[i*4+3];}backend::com(backend_device,264,reinterpret_cast<Dword*>(object[9])[8],entries);}
// Unlike creation, this entry consumes the chunk address itself as palette bytes.
void set_palette(Dword* object,Dword* image){auto* shared=reinterpret_cast<Dword*>(object[9]);shared[10]=reinterpret_cast<Dword>(find_palette(image));palette_entries(object,reinterpret_cast<const std::uint8_t*>(shared[10]));}

Dword *construct_shape(Dword *object, Dword *image) {
    base_construct(object);
    extension_construct(object + 10, object);
    Dword format = image_format(image);
    Dword *caps = backend::adapter_caps();
    int index = 0;
    for (; index < 36; ++index)
        if (caps[6912 / 4 + index] == format)
            break;
    if (index == 36)
        image = fallback();
    auto *shared = private_data(image);
    object[9] = reinterpret_cast<Dword>(shared);
    if (!shared) {
        image = fallback();
        shared = static_cast<Dword *>(eagl::allocator(56, "EAGLInternal::SharedData new"));
        if (shared)
            shared_construct(shared);
        object[9] = reinterpret_cast<Dword>(shared);
        shared[12] |= 2;
    }
    auto current_shared = [&] {
        return reinterpret_cast<Dword *>(static_cast<const volatile Dword *>(object)[9]);
    };
    shared = current_shared();
    const Dword references = shared[0] + 1;
    shared[0] = references;
    shared = current_shared();
    if (!shared[6]) {
        shared[9] = reinterpret_cast<Dword>(image);
        shared = current_shared();
        shared[1] = static_cast<std::int16_t>(image[1]);
        shared = current_shared();
        shared[2] = static_cast<std::int16_t>(image[1] >> 16);
        const Dword levels = (image[3] >> 28) + 1;
        shared = current_shared();
        shared[3] = levels;
        const Dword depth = image_depth(image);
        shared = current_shared();
        shared[4] = depth;
        const Dword format = image_format(image);
        shared = current_shared();
        shared[5] = format;
        shared = current_shared();
        shared[8] = ~0u;
        shared = current_shared();
        shared[10] = 0;
        shared = current_shared();
        auto *pitches = eagl::allocator(shared[3] * 4, "TARPrivate::mShared.mpPitch");
        shared = current_shared();
        shared[13] = reinterpret_cast<Dword>(pitches);
        Dword *palette = find_palette(image);
        shared = current_shared();
        shared[10] = reinterpret_cast<Dword>(palette);
        shared = current_shared();
        if (shared[10]) {
            const Dword sequence = *static_cast<volatile Dword *>(&palette_sequence);
            shared[8] = sequence;
            const Dword next_sequence = *static_cast<volatile Dword *>(&palette_sequence) + 1;
            *static_cast<volatile Dword *>(&palette_sequence) = next_sequence;
            shared = current_shared();
            palette = reinterpret_cast<Dword *>(shared[10]);
            const Dword palette_pixels = (palette[3] & 4096)
                                            ? reinterpret_cast<Dword>(palette) + palette[4]
                                            : reinterpret_cast<Dword>(palette + 4);
            shared[10] = palette_pixels;
            palette_entries(object, reinterpret_cast<const std::uint8_t *>(palette_pixels));
        }
        const Dword pixels = (image[3] & 4096) ? reinterpret_cast<Dword>(image) + image[4]
                                               : reinterpret_cast<Dword>(image + 4);
        shared = current_shared();
        shared[11] = pixels;
        shared = current_shared();
        Dword *device = backend_device;
        backend::com(device, 80, shared[1], shared[2], shared[3], Dword(0), shared[5], Dword(1),
                     shared + 6);
    }
    upload(object);
    return object;
}

void upload(Dword* object){auto* shared=reinterpret_cast<Dword*>(object[9]);if(shared[11]){Dword* surface=nullptr;backend::selected_device();for(int level=0;;++level){shared=reinterpret_cast<Dword*>(object[9]);backend::com(reinterpret_cast<Dword*>(shared[6]),60,level,&surface);struct Locked {Dword pitch;void* pixels;} locked;backend::com(surface,36,&locked,static_cast<const RECT*>(nullptr),Dword(2064));backend::com(surface,40);shared=reinterpret_cast<Dword*>(object[9]);reinterpret_cast<Dword*>(shared[13])[level]=locked.pitch;Dword description[8];backend::com(surface,32,description);RECT destination{0,0,static_cast<LONG>(description[6]),static_cast<LONG>(description[7])};RECT source=destination;Dword* pixels=image_level(reinterpret_cast<Dword*>(shared[9]),0,0,level);int result=surfaces::load(surface,0,&destination,pixels,reinterpret_cast<Dword*>(object[9])[5],locked.pitch,nullptr,&source,1,0);if(result<0)backend::com(surface,8);backend::com(surface,8);if(level+1>=static_cast<int>(reinterpret_cast<Dword*>(object[9])[3]))break;}}}
}
