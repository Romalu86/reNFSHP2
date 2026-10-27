#include "nfs/core/types/kernel.hpp"
namespace nfs::kernel {
#include "checksum_tables.inc"
Dword checksum16(const std::uint8_t* data,int count,std::uint16_t seed){
    Dword value=seed,index=0,entry=0;
    for(count=int(Dword(count)-1);count>=0;count=int(Dword(count)-1)){index=static_cast<std::uint8_t>(*data++^static_cast<std::uint8_t>(value^entry));
        value=(value^entry)>>8;entry=checksum16_table[index];}
    return value^entry;
}
Dword checksum32(const std::uint8_t* data,int count){
    Dword value=0;if(count>3){count-=4;value=~(Dword(data[0])<<24|Dword(data[1])<<16|Dword(data[2])<<8|data[3]);data+=4;
        while(count>0){value=(value<<8|*data++)^checksum32_table[value>>24];--count;}value=~value;}
    return value;
}
}
