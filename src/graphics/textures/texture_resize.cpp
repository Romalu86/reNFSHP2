#include "nfs/graphics/textures/texture_conversion.hpp"
#include <cstring>
namespace nfs::graphics::textures {
// Ratios are stored as float before accumulation. Intermediate arithmetic uses double.
 
 
void resize_pixels(std::uint8_t* output,const std::uint8_t* input,int width,int height,int source_width,int source_height,int bits){
 float row_step=static_cast<float>(static_cast<double>(source_height)/height),column_step=static_cast<float>(static_cast<double>(source_width)/width);
 int stride=bits>>3,bytes=bits==8?1:bits==15||bits==16?2:bits==32?4:0;if(!bytes)return;
 double row=0;for(int y=0;y<height;++y,row+=row_step){double column=0;for(int x=0;x<width;++x,column+=column_step){
  int offset=(static_cast<Dword>(static_cast<int>(column))+static_cast<Dword>(static_cast<int>(row))*source_width)*stride;
  const volatile std::uint8_t* sample=input+offset;volatile std::uint8_t* destination=output;
  std::uint8_t first=sample[0];if(bytes==1)destination[0]=first;else{std::uint8_t second=sample[1];destination[0]=first;if(bytes==2)destination[1]=second;
   else{std::uint8_t third=sample[2];destination[1]=second;std::uint8_t fourth=sample[3];destination[2]=third;destination[3]=fourth;}}output+=bytes;}}}
}
