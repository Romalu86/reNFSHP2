#include "nfs/network/session/network_virtual.hpp"
#include "nfs/graphics/resources/eagl_graphics.hpp"
namespace nfs::lifecycle::construction::network::entries {
static Dword* pointer(Dword value){return reinterpret_cast<Dword*>(value);}
static std::uint16_t short_at(Dword value,int offset=0){return *reinterpret_cast<const std::uint16_t*>(value+offset);}

void release_payload(void* data){if(data)kernel::memory_free(data);}

void __fastcall destroy_payload(Dword* object,void*,Dword data){if(data&&object[1])release_payload(pointer(data));}

Dword __fastcall payload_size(Dword* object,void*,Dword){return object[1];}
// Forward DWORD loads/stores, then forward trailing bytes.
void __fastcall copy_payload(Dword*,void*,Dword destination,Dword source,Dword bytes){for(Dword i=0;i<bytes/4;++i)pointer(destination)[i]=pointer(source)[i];
    auto* d=reinterpret_cast<std::uint8_t*>(destination);auto* s=reinterpret_cast<const std::uint8_t*>(source);for(Dword i=(bytes/4)*4;i<bytes;++i)d[i]=s[i];}
 
Dword __fastcall default_compare(Dword*,void*,Dword,Dword){return 0;}
 
Dword __fastcall default_dispatch(Dword*,void*,Dword,Dword,Dword,Dword){return 0;}
 
Dword __fastcall default_pre_dispatch(Dword*,void*,Dword,Dword,Dword){return 0;}
 
void __fastcall default_post_dispatch(Dword*,void*,Dword,Dword){}

Dword __fastcall compare_word(Dword*,void*,Dword first,Dword second){return short_at(first)==short_at(second);}

Dword __fastcall compare_second_word(Dword*,void*,Dword first,Dword second){return short_at(first)==short_at(second,2);}

Dword __fastcall compare_server_word(Dword*,void*,Dword first,Dword second){return short_at(first)==short_at(second);}

Dword __fastcall compare_vote_byte(Dword*,void*,Dword first,Dword second){return *reinterpret_cast<const std::uint8_t*>(first)==*reinterpret_cast<const std::uint8_t*>(second+2);}

Dword __fastcall vote_count_first(Dword* object,void*,Dword data,Dword,Dword,Dword vote){if(*reinterpret_cast<std::uint8_t*>(vote)==*reinterpret_cast<std::uint8_t*>(data+2))++object[76/4];return 1;}

Dword __fastcall vote_count_second(Dword* object,void*,Dword data,Dword,Dword,Dword vote){if(*reinterpret_cast<std::uint8_t*>(vote+2)==*reinterpret_cast<std::uint8_t*>(data+2)&&pointer(data)[1]==pointer(vote)[1])++object[1];return 1;}

Dword __fastcall compare_vote(Dword*,void*,Dword first,Dword second){return *reinterpret_cast<std::uint8_t*>(first+2)==*reinterpret_cast<std::uint8_t*>(second+2)&&short_at(first)==short_at(second);}

Dword __fastcall filter_message(Dword*,void*,Dword data,Dword,Dword,Dword filter){if(short_at(data,116)==9||!pointer(filter)[1])if(short_at(filter,8)!=short_at(data,118))pointer(filter)[0]=0;return 1;}
 
Dword __fastcall server_dispatch(Dword*,void*,Dword,Dword,Dword,Dword){return 1;}
}
