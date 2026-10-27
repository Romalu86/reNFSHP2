#include "nfs/physics/contacts/vehicle_contacts.hpp"
namespace nfs::game::runtime::physics {

void read_track_rotation(void *body, float *output) {
    const Dword first = at<Dword>(body, 692);
    output[2] = -at<float>(body, 700);
    at<Dword>(output, 0) = first;
    const float second_z = at<float>(body, 712);
    at<Dword>(output, 4) = at<Dword>(body, 696);
    const Dword second_x = at<Dword>(body, 704);
    output[5] = -second_z;
    const float third_x = at<float>(body, 716);
    at<Dword>(output, 12) = second_x;
    const Dword second_y = at<Dword>(body, 708);
    output[6] = -third_x;
    at<Dword>(output, 16) = second_y;
    const float third_y = at<float>(body, 720);
    at<Dword>(output, 32) = at<Dword>(body, 724);
    output[7] = -third_y;
}
} // namespace nfs::game::runtime::physics
