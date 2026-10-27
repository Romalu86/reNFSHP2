#include "nfs/graphics/backend/eagl_backend.hpp"
namespace nfs::graphics::backend {
const Dword formats16[5]={23,25,24,29,30},formats32[2]={21,22};  

Dword format_depth(Dword format){switch(format){case 20:return 24;case 21:case 22:return 32;case 23:case 24:case 25:case 26:case 29:case 30:return 16;case 27:case 28:return 8;default:return 0;}}
// The fallback chain advances only when CheckDeviceFormat fails.
std::uint8_t choose_depth(Dword* object,Dword adapter,Dword type,int* depth,int* stencil,Dword* result){if(!backend_interface)return 0;Dword format=owner(object)[3];
    auto check_format=[&](Dword candidate){return com(backend_interface,40,adapter,type,format,Dword(2),Dword(1),candidate)>=0;};
    auto match=[&](Dword candidate){return com(backend_interface,48,adapter,type,format,format,candidate)>=0;};
    auto assign=[&](Dword candidate,int bits,int s){*result=candidate;*depth=bits;*stencil=s;return std::uint8_t(1);};
    if(*depth<=16&&*stencil==0&&check_format(80)&&match(80))return assign(80,16,0);
    if(*depth<=15&&*stencil<=1&&check_format(73)&&match(73))return assign(73,15,1);
    if(*depth<=24){if(*stencil==0&&check_format(77)&&match(77))return assign(77,24,0);
        if(*depth<=24&&*stencil<=8&&check_format(75)&&match(75))return assign(75,24,8);
        if(*depth<=24&&*stencil<=4&&check_format(79)){if(match(79))return assign(79,24,4);if(check_format(80)&&match(80))return assign(80,16,0);}}
    if(*depth<=32){if(check_format(71))return match(71)?assign(71,32,0):0;
        if(check_format(75))return match(75)?assign(75,24,8):0;
        if(check_format(79))return match(79)?assign(79,24,4):0;
        if(check_format(77)&&match(77))return assign(77,24,0);}return 0;}

Dword exact_depth(Dword* object,Dword adapter,Dword format,int depth,int stencil){Dword candidate=0;
    if(depth==16&&stencil==0)candidate=80;else if(depth==15&&stencil==1)candidate=73;
    else if(depth==24){if(stencil==0)candidate=77;else if(stencil==8)candidate=75;else if(stencil==4)candidate=79;}else if(depth==32)candidate=71;
    if(!candidate)return 0;Dword type=owner(object)[164/4];if(com(backend_interface,40,adapter,type,format,Dword(2),Dword(1),candidate)<0)return 0;
    return com(backend_interface,48,adapter,type,format,format,candidate)>=0?candidate:0;}
// The windowed branch returns the display-mode format at +0Ch.
Dword choose_backbuffer(Dword* object,Dword adapter,int bits,std::uint8_t windowed){Dword type=owner(object)[164/4];if(windowed){Dword mode[4];com(backend_interface,32,adapter,mode);owner(object)[4]=mode[3];return mode[3];}
    int count=bits<=16?5:2;const Dword* formats=bits<=16?formats16:formats32;
    for(int i=0;i<count;++i)if(com(backend_interface,36,adapter,type,formats[i],formats[i],Dword(0))>=0)return formats[i];
    if(bits>16)for(int i=0;i<count;++i)if(com(backend_interface,36,adapter,type,formats16[i],formats16[i],Dword(0))>=0)return formats16[i];return 0;}

std::uint8_t populate_presentation(Dword* object){Dword* c=owner(object);presentation.count=c[56/4];presentation.width=c[8];presentation.height=c[9];presentation.window=render_window;
    if(c[7]<=3)presentation.swap=c[7]+1;presentation.multisample=c[6];return 1;}}
