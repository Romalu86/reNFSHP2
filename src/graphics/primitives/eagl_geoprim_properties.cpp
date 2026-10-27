#include "nfs/graphics/resources/eagl_assets.hpp"
#include "nfs/core/lifecycle/system_construction.hpp"
#include <cstring>
#include <cstdlib>
namespace nfs::graphics::geoprim {

void apply_property(Dword* object,const properties::Property* property){if(property->count!=1)return;const char* name=property->name;const char* value=property->values[0];
    if(!std::strcmp(name,"SetPrimitiveType"))primitive(object,properties::geo_value(value));
    else if(!std::strcmp(name,"SetShading"))shading(object,properties::geo_value(value));
    else if(!std::strcmp(name,"SetCullEnable"))culling(object,!std::strcmp(value,"true"));
    else if(!std::strcmp(name,"SetTextureEnable"))texture_enable(object,!std::strcmp(value,"true"));
    else if(!std::strcmp(name,"SetTextureCoordType"))texture_coordinates(object,properties::geo_value(value));
    else if(!std::strcmp(name,"SetDepthTestMethod"))depth_method(object,properties::geo_value(value));
    else if(!std::strcmp(name,"SetTransparencyMethod"))transparency(object,properties::geo_value(value));
    else if(!std::strcmp(name,"SetChromaColour"))chroma_colour(object,reinterpret_cast<Dword>(property));
    else if(!std::strcmp(name,"SetAlphaBlendMode"))alpha_blend_mode(object,properties::geo_value(value));
    else if(!std::strcmp(name,"SetAlphaTestEnable"))alpha_test(object,!std::strcmp(value,"true"));
    else if(!std::strcmp(name,"SetAlphaCompareValue"))alpha_reference(object,static_cast<Dword>(std::atoi(value)));
    else if(!std::strcmp(name,"SetAlphaTestMethod"))alpha_method(object,properties::geo_value(value));}
// UID alone controls ownership; allocator's context output is untouched.
Dword* runtime_allocate(const char* text,eagl::Loader*,Dword*,std::uint8_t* owned){properties::Parser parser;properties::parse(&parser,text);
    auto* object=static_cast<Dword*>(eagl::allocator(68,"EAGL::GeoPrimState new"));if(object)lifecycle::construction::render_state(object);
    for(int i=0;i<parser.count;++i){const auto& property=parser.entries[i];const char* name=property.name;
        if(!std::strcmp(name,"UID")){*owned=1;continue;}
        if(std::strncmp("PCEXTOBJ",name,8)){apply_property(object,&property);continue;}
        if(property.count==1&&!std::strcmp(name,"PCEXTOBJ_SetCullDirection"))cull_direction(object,properties::pc_value(property.values[0]));
        else if(property.count==1&&!std::strcmp(name,"PCEXTOBJ_SetFillMode"))fill_mode(object,properties::pc_value(property.values[0]));
        else if(!std::strcmp(name,"PCEXTOBJ_SetBlendOperation,1"))blend_operation(object,properties::pc_value(property.values[0]));
        else if(property.count==3&&!std::strcmp(name,"PCEXTOBJ_SetAlphaBlend")){
            Dword operation=properties::pc_value(property.values[2]),destination=properties::pc_value(property.values[1]),source=properties::pc_value(property.values[0]);blend_factors(object,source,destination,operation);}
        else if(property.count==1){if(!std::strcmp(name,"PCEXTOBJ_SetNumPatchSegments")){float value=static_cast<float>(std::atof(property.values[0]));Dword bits;std::memcpy(&bits,&value,4);patch_segments(object,bits);}
            else if(!std::strcmp(name,"PCEXTOBJ_SetZWritesEnable"))depth_write(object,!std::strcmp(property.values[0],"true"));}}
    properties::destroy(&parser);return object;}
}
