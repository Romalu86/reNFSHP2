#include "nfs/graphics/resources/eagl_assets.hpp"
namespace nfs::graphics::methods {
Dword current_draw=0;

std::uint8_t command_draw_marker(Dword* method,Dword,Dword index){Dword* cursor=reinterpret_cast<Dword*>(method[0]);if(current_draw!=method[4]){current_draw=method[4];return 1;}while(cursor[0]||cursor[1]){Dword step=*cursor&65535,kind=*cursor>>16;switch(kind){case 2:case 10:case 12:if(static_cast<int>(index)>0)*cursor=step|65536;break;case 16:case 17:case 18:step=2;break;default:break;}if((*cursor>>16)==0)step=1;cursor+=step;}return 1;}

void model_initialize(Dword* model){Dword* output=reinterpret_cast<Dword*>(model[204/4]);Dword header=*output;Dword* input=output+1;Dword threshold=header&65535;Dword* count_location=nullptr;Dword count=0,index=0;for(;;){Dword* tail;if((header&65535)==65535){if(header&268435456)return;input[-1]=header|268435456;Dword next=index+1;if(command_draw_marker(reinterpret_cast<Dword*>(*reinterpret_cast<Dword*>(*input)),*input,index)){output[0]=input[-1];output[1]=*input;output+=2;count+=2;}index=next;tail=input;}else{if(count_location)*count_location=count;output[0]=input[-1];count=0;output[1]=*input;count_location=output+2;*count_location=input[1];output+=3;tail=input+1;index=0;}header=tail[1];input=tail+2;if(threshold>=(header&65535)){*output=header;if(count_location)*count_location=count;return;}}}

void model_constructor(Dword* object){model_initialize(object);}
 
void model_destructor(Dword*){}
}
