#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstdio>
namespace nfs::graphics::backend {
Capabilities capability_data{};
Dword* hardware_caps=nullptr;Dword* alternate_caps=nullptr;  
const Dword texture_formats[36]={20,21,22,23,24,25,26,27,28,29,30,40,41,50,51,52,60,61,62,63,64,65,1498831189,844715353,827611204,844388420,861165636,877942852,894720068,70,71,73,75,80,77,79};  

void enumerate_modes(Dword* direct3d,Dword* caps){int used=0;caps[764/4]=0;int count=com(direct3d,24,caps[528/4]);
    for(int i=0;i<count;++i){Dword mode[4];com(direct3d,28,caps[528/4],i,mode);if(used<256){int found=0;
        for(;found<used;++found){Dword* entry=caps+768/4+found*6;if(entry[0]==1&&entry[1]==mode[0]&&entry[2]==mode[1]&&entry[5]==mode[3]){
            if(static_cast<int>(entry[3])<static_cast<int>(mode[2]))entry[3]=mode[2];if(static_cast<int>(mode[2])<static_cast<int>(entry[4]))entry[4]=mode[2];break;}}
        if(found==used){Dword* entry=caps+768/4+used*6;entry[0]=1;entry[1]=mode[0];entry[2]=mode[1];entry[4]=mode[2];entry[3]=mode[2];entry[5]=mode[3];++used;}}}caps[764/4]=used;}

void enumerate_formats(Dword* direct3d,Dword* caps){for(int i=0;i<36;++i){caps[6912/4+i]=0;Dword format=texture_formats[i];
    if(com(direct3d,40,caps[528/4],Dword(1),caps[544/4],Dword(0),Dword(3),format)==0)caps[6912/4+i]=format;}}
// Exact adapter-ID exceptions, including signed capability fields.
void apply_device_quirks(Dword* caps){Dword flags=caps[760/4];flags=(flags&~Dword(16))|((!(caps[592/4]&16)||!(caps[596/4]&2))?16:0);caps[760/4]=flags;Dword id=caps[516/4];
    auto force_one=[&]{caps[700/4]=1;};auto no_anisotropy=[&]{caps[584/4]=0;};
    if(id>=282984704&&id<=282984707)return;
    switch(id){case 268586057:case 268584777:case 268584770:case 268584772:case 268586050:case 268586052:case 268586055:force_one();no_anisotropy();return;
    case 272196390:case 271254816:case 271254817:case 303693827:case 303693828:case 303693829:return;
    case 271777834:case 271777862:force_one();caps[760/4]|=16;return;
    case 273431815:case 1027407881:force_one();caps[760/4]|=32;no_anisotropy();return;
    case 282984480:case 1027407882:case 303693826:caps[760/4]=flags|1;return;
    case 303693825:force_one();caps[760/4]|=1;return;
    case 315752472:case 315752473:force_one();caps[760/4]|=48;no_anisotropy();return;
    case 1395886624:case 1395886625:caps[760/4]=(flags&~Dword(4))|3;return;
    case 1395886626:caps[760/4]=flags|3;return;
    case 3997905:case 285933572:case 1395873329:case 1395886593:case 2156296192u:force_one();return;
    case 1395886141:force_one();return;
    case 1395888386:caps[760/4]=flags|67;caps[664/4]=caps[672/4]=caps[668/4]=caps[676/4]=0;return;default:break;}
    if(id>=282984488&&id<=282984495){caps[760/4]=flags|1;return;}
    if((id>268587072&&id<268587263)||(id>268587584&&id<268587775)||(id>268587840&&id<268588031)){no_anisotropy();return;}
    if((id>=268584781&&id<=268584785)||(id>=268586057&&id<=268586067)){no_anisotropy();force_one();}}
// Release GetDirect3D's acquired reference before the following calls.
Dword* retrieve_capabilities(Dword* device,char* error){Dword* caps=capability_data.words;for(Dword& value:capability_data.words)value=0;
    Dword* direct3d=nullptr;Dword creation[4]{};Dword identifier[267]{};caps[0]=0;caps[528/4]=0;
    if(!device){if(error)std::sprintf(error,"pDevice is NULL\n");return nullptr;}
    int result=com(device,24,&direct3d);if(result){if(error)std::sprintf(error,"IDirect3DDevice8_GetDirect3D failed: err = 0x%x\n",result);return nullptr;}
    com(direct3d,8);result=com(device,36,creation);if(result){if(error)std::sprintf(error,"IDirect3DDevice8_GetCreationParameters failed: err = 0x%x\n",result);return nullptr;}
    caps[528/4]=creation[0];result=com(direct3d,20,creation[0],Dword(2),identifier);if(result){if(error)std::sprintf(error,"IDirect3D8_GetAdapterIdentifier failed: err = 0x%x\n",result);return nullptr;}
    caps[520/4]=identifier[1032/4];caps[524/4]=identifier[1036/4];caps[516/4]=(identifier[1036/4]&65535)|(identifier[1032/4]<<16);
    auto* description=reinterpret_cast<char*>(identifier)+512;auto* output=reinterpret_cast<char*>(caps)+4;int i=0;do{output[i]=description[i];}while(description[i++]);
    enumerate_modes(direct3d,caps);com(device,32,caps+532/4);enumerate_formats(direct3d,caps);result=com(device,28,caps+548/4);
    if(result){if(error)std::sprintf(error,"IDirect3DDevice8_GetDeviceCaps failed: err = 0x%x\n",result);return nullptr;}apply_device_quirks(caps);caps[0]=1;return caps;}

// Authorized display compatibility path. Adapter queries require no rendering
// device, swap chain, window, mode switch, or additional COM reference.
Dword *query_adapter_capabilities(Dword *direct3d, Dword adapter, char *error) {
    Dword *caps = capability_data.words;
    for (Dword &value : capability_data.words)
        value = 0;
    if (!direct3d) {
        if (error)
            std::sprintf(error, "pDirect3D is NULL\n");
        return nullptr;
    }

    Dword identifier[267]{};
    int result = com(direct3d, 20, adapter, Dword(2), identifier);
    if (result) {
        if (error)
            std::sprintf(error, "IDirect3D8_GetAdapterIdentifier failed: err = 0x%x\n", result);
        return nullptr;
    }
    caps[528 / 4] = adapter;
    caps[520 / 4] = identifier[1032 / 4];
    caps[524 / 4] = identifier[1036 / 4];
    caps[516 / 4] = (identifier[1036 / 4] & 65535) | (identifier[1032 / 4] << 16);
    const char *description = reinterpret_cast<const char *>(identifier) + 512;
    char *output = reinterpret_cast<char *>(caps) + 4;
    int index = 0;
    do {
        output[index] = description[index];
    } while (description[index++]);

    enumerate_modes(direct3d, caps);
    result = com(direct3d, 32, adapter, caps + 532 / 4);
    if (result) {
        if (error)
            std::sprintf(error, "IDirect3D8_GetAdapterDisplayMode failed: err = 0x%x\n", result);
        return nullptr;
    }
    enumerate_formats(direct3d, caps);
    result = com(direct3d, 52, adapter, Dword(1), caps + 548 / 4);
    if (result) {
        if (error)
            std::sprintf(error, "IDirect3D8_GetDeviceCaps failed: err = 0x%x\n", result);
        return nullptr;
    }
    apply_device_quirks(caps);
    caps[0] = 1;
    return caps;
}
}
