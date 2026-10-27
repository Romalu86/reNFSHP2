#include "nfs/graphics/backend/eagl_backend.hpp"
#include <cstring>
namespace nfs::graphics::backend {
static_assert(__LDBL_MANT_DIG__ == 64 && __LDBL_MAX_EXP__ == 16384,
              "Backend float state requires native x87 intermediates");
namespace {
__declspec(noinline) Dword float_argument_bits(const float *input) {
    Dword bits;
    std::memcpy(&bits, input, 4);
    return bits;
}
}

std::uint8_t set_depth_write(Dword* object,std::uint8_t value){reinterpret_cast<std::uint8_t*>(object)[109]=value;if(depth_write_state!=value){depth_write_state=value;if(backend_device)com(backend_device,200,Dword(14),Dword(reinterpret_cast<std::uint8_t*>(object)[109]));}return 1;}

std::uint8_t set_stencil_depth_fail(Dword* object,Dword value){owner(object)[76/4]=value;if(backend_device)com(backend_device,200,Dword(54),value);return 1;}

std::uint8_t set_stencil_pass(Dword* object,Dword value){owner(object)[80/4]=value;if(backend_device)com(backend_device,200,Dword(55),value);return 1;}

std::uint8_t set_stencil_fail(Dword* object,Dword value){owner(object)[84/4]=value;if(backend_device)com(backend_device,200,Dword(53),value);return 1;}

std::uint8_t set_stencil_function(Dword* object,Dword value){owner(object)[88/4]=value;if(backend_device)com(backend_device,200,Dword(56),value);return 1;}

std::uint8_t set_stencil_reference(Dword* object,Dword value){owner(object)[92/4]=value;if(backend_device)com(backend_device,200,Dword(57),value);return 1;}

std::uint8_t set_stencil_mask(Dword* object,Dword value){owner(object)[96/4]=value;if(backend_device)com(backend_device,200,Dword(58),value);return 1;}

std::uint8_t set_stencil_write_mask(Dword* object,Dword value){owner(object)[100/4]=value;if(backend_device)com(backend_device,200,Dword(59),value);return 1;}

std::uint8_t set_stencil_enable(Dword* object,std::uint8_t value){reinterpret_cast<std::uint8_t*>(owner(object))[108]=value;if(backend_device)com(backend_device,200,Dword(52),Dword(value));return 1;}

std::uint8_t set_fog_density(Dword *object, float value) {
    Dword *state = owner(object);
    const Dword input_bits = float_argument_bits(&value);
    const volatile long double retained_input = static_cast<long double>(value);
    state[128 / 4] = input_bits;
    Dword *device = *static_cast<Dword *volatile *>(&backend_device);
    const float negative = float(-retained_input);
    Dword negative_bits;
    std::memcpy(&negative_bits, &negative, 4);
    const volatile Dword stored_negative = negative_bits;
    if (device)
        com(device, 200, Dword(38), Dword(stored_negative));
    return 1;
}

std::uint8_t set_fog_colour(Dword* object,Dword value){owner(object)[132/4]=value;if(backend_device)com(backend_device,200,Dword(34),value);return 1;}
}
