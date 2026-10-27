#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::backend {

Dword texture_format(Dword* object,Dword bits){Dword* caps=adapter_caps();const Dword* formats;int count;const Dword format8=27,format24=20;
    switch(bits){case 8:formats=&format8;count=1;break;case 16:formats=formats16;count=5;break;case 24:formats=&format24;count=1;break;case 32:formats=formats32;count=2;break;default:return 0;}
    for(int i=0;i<count;++i)for(int j=0;j<36;++j)if(caps[6912/4+j]==formats[i]){Dword* context=owner(object);
        if(com(backend_interface,40,context[140/4],context[164/4],context[3],Dword(1),Dword(3),formats[i])==0)return formats[i];}return 0;}

std::uint8_t windowed_formats(Dword* object,std::uint8_t){com(backend_interface,32,owner(object)[140/4],desktop_mode);presentation.windowed=1;
    owner(object)[4]=desktop_mode[3];presentation.format=desktop_mode[3];owner(object)[3]=presentation.format;Dword* context=owner(object);
    if(context[52/4]+context[48/4]!=format_depth(presentation.format)||!choose_depth(object,context[140/4],context[164/4],reinterpret_cast<int*>(context+52/4),reinterpret_cast<int*>(context+48/4),context+5))owner(object)[52/4]=format_depth(presentation.format);
    owner(object)[40/4]=format_depth(presentation.format);owner(object)[44/4]=owner(object)[40/4];presentation.refresh=0;presentation.interval=0;return 1;}
// A 32-bit request uses only two 16-bit fallback formats.
std::uint8_t fullscreen_formats(Dword* object,std::uint8_t probing){Dword* context=owner(object);const Dword* formats=static_cast<int>(context[44/4])<=16?formats16:formats32;int count=static_cast<int>(context[44/4])<=16?5:2;Dword adapter_format=0,buffer_format=0;
    auto find=[&](const Dword* values){for(int a=0;a<count;++a){for(int b=0;b<count;++b){owner(object)[164/4]=1;
        if(com(backend_interface,36,owner(object)[140/4],Dword(1),values[a],values[b],Dword(0))>=0){adapter_format=values[a];buffer_format=values[b];break;}}if(adapter_format&&buffer_format)break;}};
    find(formats);if(static_cast<int>(owner(object)[44/4])>16&&!adapter_format){find(formats16);if(!adapter_format||!buffer_format)return 0;owner(object)[40/4]=owner(object)[44/4]=16;}
    presentation.windowed=0;owner(object)[3]=adapter_format;owner(object)[4]=buffer_format;presentation.format=buffer_format;
    if(probing)presentation.refresh=0;else{Dword* caps=adapter_caps();Dword refresh_mode=owner(object)[148/4];
        if(refresh_mode==1||refresh_mode==2){for(int i=0;i<256;++i){Dword* entry=caps+768/4+i*6;Dword* c=owner(object);
            if(entry[0]==1&&entry[1]==c[8]&&entry[2]==c[9]&&entry[5]==c[4])presentation.refresh=entry[refresh_mode==1?4:3];}}else presentation.refresh=0;
        caps=adapter_caps();if(caps){if(!reinterpret_cast<std::uint8_t*>(owner(object))[60]&&static_cast<int>(caps[568/4])<0){presentation.interval=2147483648u;return 1;}if(!(caps[568/4]&1))return 0;}}
    presentation.interval=1;return 1;}
}
