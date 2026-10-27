#include "nfs/graphics/textures/surface_conversion.hpp"
#include <cstring>
namespace nfs::graphics::surfaces {
DebugMute mute_runtime=nullptr,mute_debug=nullptr;
// Module discovery is called only by the game-controlled caller.
void debug_mute(Dword value){if(!mute_runtime){if(mute_debug){mute_debug(value);return;}
 if(GetModuleHandleA("d3d8.dll")){if(HMODULE module=LoadLibraryA("d3d8.dll"))mute_runtime=reinterpret_cast<DebugMute>(GetProcAddress(module,"DebugSetMute"));}
 if(GetModuleHandleA("d3d8d.dll")){if(HMODULE module=LoadLibraryA("d3d8d.dll"))mute_debug=reinterpret_cast<DebugMute>(GetProcAddress(module,"DebugSetMute"));}}
 if(mute_runtime)mute_runtime(value);if(mute_debug)mute_debug(value);}

SurfaceMap* construct(SurfaceMap* object){object->flags=0;object->surface=nullptr;object->temporary=nullptr;object->device=nullptr;return object;}
int unlock(SurfaceMap* object){Dword* locked=object->temporary?object->temporary:object->surface;if(locked)backend::com(locked,40);
 if(object->surface&&object->temporary&&object->device&&!(object->flags&1))backend::com(object->device,112,object->temporary,Dword(0),Dword(0),object->surface,Dword(0));
 if(object->temporary){backend::com(object->temporary,8);object->temporary=nullptr;}if(object->device){backend::com(object->device,8);object->device=nullptr;}object->surface=nullptr;return 0;}
int destroy(SurfaceMap* object){return unlock(object);}

VolumeMap* construct(VolumeMap* object){object->volume=nullptr;return object;}int unlock(VolumeMap* object){if(object->volume)backend::com(object->volume,40);object->volume=nullptr;return 0;}
int destroy(VolumeMap* object){return unlock(object);}
static const GUID texture_iid={3838694773u,10342,20225,{177,46,126,236,225,236,147,88}};
static const GUID render_iid={3022068986u,20921,19103,{171,120,219,153,178,187,103,142}};
static bool compressed(Dword format){return format==827611204||format==844388420||format==861165636||format==877942852||format==894720068;}
// COM reference acquisition, failed-copy cleanup and aligned partial locks are kept.
int map(SurfaceMap* object,Image* output,Dword* surface,Dword palette,const RECT* area,Dword key,Dword flags){unlock(object);
 Dword description[8];backend::com(surface,32,description);Dword width=description[6],height=description[7];RECT requested;
 bool partial=false,discard;if(area){requested=*area;if(requested.left<0||width<static_cast<Dword>(requested.right)||requested.right<requested.left||requested.top<0||height<static_cast<Dword>(requested.bottom)||requested.bottom<requested.top)return static_cast<int>(2289436780u);
 partial=requested.left!=0||static_cast<Dword>(requested.right)!=width||requested.top!=0||static_cast<Dword>(requested.bottom)!=height;discard=!(flags&1)&&!partial;
 }else{requested={0,0,static_cast<LONG>(width),static_cast<LONG>(height)};discard=!(flags&1);}
 Dword use_temporary=flags&65536;Dword usage=description[2];Dword* locked=surface;
 if(!use_temporary&&!description[3]&&!(usage&512)){debug_mute(1);Dword* container=nullptr;int result=backend::com(surface,28,&render_iid,&container);
  if(result<0){usage=surface[4];use_temporary=~(usage>>26)&1;}else{use_temporary=1;if(container)backend::com(container,8);}debug_mute(0);}
 if(use_temporary){Dword aligned_width=width,aligned_height=height;if(compressed(description[0])){aligned_width=(width+3)&~3u;aligned_height=(height+3)&~3u;}
  backend::com(surface,12,&object->device);int result=backend::com(object->device,108,aligned_width,aligned_height,description[0],&object->temporary);if(result<0)return result;
  if(!discard){result=backend::com(object->device,112,surface,Dword(0),Dword(0),object->temporary,Dword(0));if(result<0)return result;}locked=object->temporary;}
 Dword lock_flags=((flags&1)|128)<<4;
 if(discard&&!use_temporary&&(usage&512)){if(flags&131072)lock_flags|=8192;
  else{debug_mute(1);Dword* container=nullptr;int result=backend::com(surface,28,&texture_iid,&container);if(result>=0){if(backend::com(container,52)==1){lock_flags|=8192;partial=false;}if(container)backend::com(container,8);}debug_mute(0);}}
 struct Locked {Dword pitch,data;} lock;int result;RECT aligned;
 if(partial&&!(discard&&!use_temporary&&(usage&512)&&(flags&131072))){aligned=requested;
  if(compressed(description[0])){aligned.left&=~3;aligned.top&=~3;aligned.right=(static_cast<Dword>(aligned.right)+3)&~3u;aligned.bottom=(static_cast<Dword>(aligned.bottom)+3)&~3u;
   if(!use_temporary){if(width<static_cast<Dword>(aligned.right))aligned.right=width;if(height<static_cast<Dword>(aligned.bottom))aligned.bottom=height;}
  }else if(description[0]==844715353||description[0]==1498831189){aligned.left&=~1;aligned.right=(static_cast<Dword>(aligned.right)+1)&~1u;if(width<static_cast<Dword>(aligned.right))aligned.right=width;}
  Dword full_width=use_temporary&&compressed(description[0])?(width+3)&~3u:width,full_height=use_temporary&&compressed(description[0])?(height+3)&~3u:height;
  if(aligned.left==0&&static_cast<Dword>(aligned.right)==full_width&&aligned.top==0&&static_cast<Dword>(aligned.bottom)==full_height)partial=false;
 }
 if(partial){result=backend::com(locked,36,&lock,&aligned,lock_flags);if(result<0)return result;requested.left-=aligned.left;requested.right-=aligned.left;requested.top-=aligned.top;requested.bottom-=aligned.top;
 }else{result=backend::com(locked,36,&lock,static_cast<const RECT*>(nullptr),lock_flags);if(result<0)return result;}
 Dword* image=output->words;image[0]=lock.data;image[1]=description[0];image[2]=lock.pitch;image[3]=image[4]=image[5]=image[8]=image[14]=0;
 image[6]=width;image[7]=height;image[9]=image[15]=image[16]=1;std::memcpy(image+10,&requested,16);image[17]=key;image[18]=palette;object->surface=surface;object->flags=flags;return 0;}
}
