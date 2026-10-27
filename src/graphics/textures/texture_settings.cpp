#include "nfs/graphics/textures/texture_conversion.hpp"
#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstdlib>
#include <cstring>
namespace nfs::graphics::textures {
TextureReader database{};
std::uint8_t conversion_enabled=1;
const Format formats[12]={
 {20,"D3DFMT_R8G8B8",24,127},{21,"D3DFMT_A8R8G8B8",32,125},
 {23,"D3DFMT_R5G6B5",16,120},{25,"D3DFMT_A1R5G5B5",16,126},
 {26,"D3DFMT_A4R4G4B4",16,109},{27,"D3DFMT_R3G3B2",8,120},
 {28,"D3DFMT_A8",8,100},{31,"D3DFMT_A2B10G10R10",32,106},
 {41,"D3DFMT_P8",8,123},{827611204,"D3DFMT_DXT1",32,96},
 {861165636,"D3DFMT_DXT3",32,97},{894720068,"D3DFMT_DXT5",32,98}};
Fallback indexed_fallback[3]={{0,0},{1,0},{2,0}};
Fallback packed_fallback[3]={{0,0},{1,0},{2,0}};
Fallback colour_fallback[3]={{0,0},{1,0},{2,0}};
static void reset(TextureReader* object){Dword* f=object->fields;f[0]=f[1]=0;f[2]=2;
 auto* b=reinterpret_cast<std::uint8_t*>(f);b[12]=b[13]=0;f[4]=f[5]=f[6]=0;f[11]=f[12]=f[13]=0;std::memset(f+14,0,144);}
static void link(TextureReader* object){database.fields[7]=reinterpret_cast<Dword>(object->fields+8);object->fields[8]=reinterpret_cast<Dword>(&database);
 auto* b=reinterpret_cast<std::uint8_t*>(object->fields);b[36]=b[37]=b[38]=b[39]=b[40]=0;}
static void unlink(TextureReader* object){auto* owner=reinterpret_cast<Dword*>(object->fields[8]);
 owner[7]=reinterpret_cast<Dword>(owner+8);}
// Embedded registration is retained, including the global alias at +1Ch.
TextureReader* construct(TextureReader* object,const char* name,const char* directory){link(object);
 try{reset(object);if(!read_settings(object,name,directory))object->fields[2]=0;reserve(object,5242880);}
 catch(...){unlink(object);throw;}return object;}
TextureReader* construct(TextureReader* object){link(object);
 try{reset(object);reserve(object,5242880);}catch(...){unlink(object);throw;}return object;}
// Free uses the CRT heap, not EAGL's allocator callback.
void destroy(TextureReader* object){struct RegistrationCleanup{TextureReader* object;~RegistrationCleanup(){unlink(object);}} cleanup{object};
 if(object->fields[0])std::free(reinterpret_cast<void*>(object->fields[0]));reset(object);}
// Failed growth loses old contents and retries the previous capacity.
Dword reserve(TextureReader* object,Dword requested){Dword previous=object->fields[1];if(requested<16)requested=524288;else if(requested<previous)requested=previous;
 if(requested<=previous)return 0;if(object->fields[0])std::free(reinterpret_cast<void*>(object->fields[0]));object->fields[1]=requested;
 object->fields[0]=reinterpret_cast<Dword>(std::malloc(requested));if(object->fields[0])return 0;object->fields[1]=previous;
 object->fields[0]=reinterpret_cast<Dword>(std::malloc(previous));if(!object->fields[0])object->fields[1]=0;return 8;}
// Compare corresponding format slots in order, not sets or mCrc.
bool equivalent(const TextureReader* a,const TextureReader* b){const auto* x=reinterpret_cast<const std::uint8_t*>(a);const auto* y=reinterpret_cast<const std::uint8_t*>(b);
 if(a->fields[2]!=b->fields[2]||x[12]!=y[12]||x[13]!=y[13]||a->fields[4]!=b->fields[4]||a->fields[5]!=b->fields[5])return false;
 for(int outer=0;outer<36;++outer){bool found=false;for(int i=0;i<36&&!found;++i)found=a->fields[14+i]==b->fields[14+i];if(!found)return false;}return true;}

bool different(const TextureReader* a,const TextureReader* b){return !equivalent(a,b);}

bool supports_format(const TextureReader* object,Dword format){if(format)for(int i=0;i<36;++i)if(object->fields[14+i]==format)return true;return false;}

bool supports_shape(const TextureReader* object,Dword shape){Dword format=23;for(const auto& entry:formats)if(entry.shape==(shape&127)){format=entry.format;break;}return supports_format(object,format);}
bool supports_dxt3(const TextureReader* object){return supports_shape(object,97);}
// Each of the three tables defines a separate format preference order.
void select_fallbacks(TextureReader* object){Dword last=supports_format(object,25)?25:23;
 auto choose=[&](const Dword* order,int count,Dword fallback){for(int i=0;i<count;++i)if(supports_format(object,order[i]))return order[i];return fallback;};
 for(auto& entry:colour_fallback){Dword opaque[]={23,21,26},binary[]={25,21,26},smooth[]={21,26,25};
  entry.format=entry.alpha==1?choose(binary,3,last):(entry.alpha==2||entry.alpha==3)?choose(smooth,3,last):choose(opaque,3,last);}
 for(auto& entry:packed_fallback){Dword opaque[]={23,25,21,26},binary[]={25,26,21},smooth[]={21,26,25};
  entry.format=entry.alpha==1?choose(binary,3,last):(entry.alpha==2||entry.alpha==3)?choose(smooth,3,last):choose(opaque,4,last);}
 for(auto& entry:indexed_fallback){Dword opaque[]={41,23,21,26},binary[]={41,25,21,26},smooth[]={41,21,26};
  entry.format=entry.alpha==1?choose(binary,4,last):(entry.alpha==2||entry.alpha==3)?choose(smooth,3,last):choose(opaque,4,last);}}
// A missing integer leaves the same previous stack value; it is not defaulted to zero here.
Dword read_settings(TextureReader* object,const char* name,const char* directory){reset(object);gui::String path;gui::string_construct(&path);ini::Reader reader;
 try{gui::string_assign(&path,directory);gui::string_append_ascii(&path,"\\");gui::string_append_ascii(&path,name);ini::reader_construct(&reader,gui::string_ascii(&path),0);
  try{int value;ini::get_int(&reader,"General","mPowerOf2",&value);reinterpret_cast<std::uint8_t*>(object)[12]=value!=0;
   ini::get_int(&reader,"General","mSquareOnly",&value);reinterpret_cast<std::uint8_t*>(object)[13]=value!=0;
   ini::get_bool(&reader,"General","mMaxTextureWidth",object->fields+4);ini::get_bool(&reader,"General","mMaxTextureHeight",object->fields+5);ini::get_bool(&reader,"General","mCrc",object->fields+6);
   Dword* output=object->fields+14;for(const auto& entry:formats){ini::get_int(&reader,"Format",entry.name,&value);if(value==1)*output++=entry.format;}
  }catch(...){ini::reader_clear(&reader);throw;}ini::reader_clear(&reader);
 }catch(...){gui::string_destruct(&path);throw;}gui::string_destruct(&path);return 0;}
// Saving invokes the INI reader's real modified/save lifecycle.
Dword write_settings(TextureReader* object,const char* name,const char* directory){if(object->fields[2])return 2;gui::String path;gui::string_construct(&path);ini::Reader reader;
 try{gui::string_assign(&path,directory);gui::string_append_ascii(&path,"\\");gui::string_append_ascii(&path,name);ini::reader_construct(&reader,gui::string_ascii(&path),0);
  try{ini::set_int(&reader,"General","mPowerOf2",reinterpret_cast<const std::uint8_t*>(object)[12]!=0);
   ini::set_int(&reader,"General","mSquareOnly",reinterpret_cast<const std::uint8_t*>(object)[13]!=0);
   ini::set_word(&reader,"General","mMaxTextureWidth",object->fields[4]);ini::set_word(&reader,"General","mMaxTextureHeight",object->fields[5]);ini::set_word(&reader,"General","mCrc",object->fields[6]);
   for(const auto& entry:formats){bool found=false;for(int i=0;i<36&&!found;++i)found=object->fields[14+i]==entry.format;ini::set_int(&reader,"Format",entry.name,found);}
  }catch(...){ini::reader_clear(&reader);throw;}ini::reader_clear(&reader);
 }catch(...){gui::string_destruct(&path);throw;}gui::string_destruct(&path);return 0;}
}
