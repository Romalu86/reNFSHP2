#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cstring>
#include <intrin.h>
namespace nfs::graphics::tar {
#include "eagl_embedded_shapes.inc"
#include "eagl_image_tables.inc"

Dword image_format(const Dword* image){if(!image)return 0;Dword value;switch(*image&127){case 96:value=827611204;break;case 97:value=861165636;break;case 98:value=894720068;break;case 100:case 109:value=26;break;case 120:value=23;break;case 123:value=41;break;case 125:value=21;break;case 126:value=25;break;case 127:value=20;break;default:return 0;}return value|((image[3]&8192)?2147483648u:0);}

Dword storage_format(int value){if(value<42){switch(value){case 20:return 127;case 21:case 22:return 125;case 23:return 120;case 24:case 25:return 126;case 26:case 28:case 30:return 109;case 41:return 123;case 27:case 29:return 0;default:return 0;}}switch(value){case 827611204:return 96;case 861165636:return 97;case 894720068:return 98;default:return 0;}}

Dword* find_palette(Dword* image){while(image){if((*image&255)==42)return image;int step=static_cast<int>(*image)>>8;if(!step)break;image=reinterpret_cast<Dword*>(reinterpret_cast<Dword>(image)+step);}return nullptr;}

Dword* image_private(Dword* image){while(image){if((*image&255)==105)return (reinterpret_cast<std::uint8_t*>(image)[6]&16)?image+4:nullptr;int step=static_cast<int>(*image)>>8;if(!step)return nullptr;image=reinterpret_cast<Dword*>(reinterpret_cast<Dword>(image)+step);}return nullptr;}

Dword *private_data(Dword *image) {
    Dword *marker = image_private(image);
    if (!marker)
        return nullptr;
    Dword *output = marker + 1;
    auto *signature = reinterpret_cast<volatile std::uint8_t *>(marker);
    if (signature[0] != 'U' || signature[1] != 'S' || signature[2] != 'E' ||
        signature[3] != 'D') {
        signature[0] = 'U';
        signature[1] = 'S';
        signature[2] = 'E';
        signature[3] = 'D';
        if (output)
            shared_construct(output);
        __stosd(reinterpret_cast<unsigned long *>(output), 0, 14);
    }
    return output;
}
}
