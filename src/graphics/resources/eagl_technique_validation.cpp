#include "nfs/graphics/resources/eagl_assets.hpp"
#include <cassert>
namespace nfs::graphics::techniques {
#include "../loader/eagl_generation_messages.inc"
static Dword capability(unsigned address){return device_caps[(address-8692056)/4];}

std::uint8_t parameter_validate(Dword* parameter){switch(parameter[2]){case 3:return parameter[1]<capability(8692204);case 4:{Dword* stages=reinterpret_cast<Dword*>(parameter[0]);if(capability(8692204)<=stages[136])return 0;Dword operation=stages[0];if(operation>=1&&operation<=26)return static_cast<std::uint8_t>((capability(8692200)>>(operation-1))&1);_assert(invalid_expression,technique_filename,449);return 0;}case 5:{Dword bits=0,maximum=0,line=0;switch(parameter[0]){case 16:bits=capability(8692096);maximum=8;line=531;break;case 32:bits=capability(8692108);maximum=8;line=555;break;case 48:return static_cast<std::uint8_t>((capability(8692088)>>11)&1);case 52:bits=capability(8692100);maximum=13;line=591;break;case 56:bits=capability(8692104);maximum=13;line=625;break;default:return 1;}Dword value=parameter[3];if(value>=1&&value<=maximum)return static_cast<std::uint8_t>((bits>>(value-1))&1);_assert(invalid_expression,technique_filename,line);return 0;}case 6:case 8:return parameter[3]!=0;case 7:return parameter[3]!=~0u;default:_assert(invalid_expression,technique_filename,639);return 1;}}
}
