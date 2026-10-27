#include "../../physics/vehicles/car_construction_runtime.hpp"
#include "nfs/world/lifecycle/world_lifecycle.hpp"
namespace nfs::application::pending {
struct CarRunStatistics;
void destroy_car_run_statistics(CarRunStatistics *);
} // namespace nfs::application::pending
namespace nfs::game::world::pending {

void *construct_car(void *car) {
    using namespace runtime::physics::construction;
    auto *car_words = static_cast<Dword *>(car);
    construct_scalar_parameters(offset(car, 4));
    construct_response_parameters(offset(car, 68));
    construct_curve_parameters(offset(car, 104));
    construct_pose_parameters(offset(car, 364));
    for (int word = 124; word >= 119; --word)
        car_words[word] = 0;
    lifecycle::construction::objects::vehicle_dynamics(offset(car, 500));
    car_words[391] = car_words[392] = car_words[393] = 0;
    const Dword zero = 0;
    for (unsigned field = 1628; field <= 1676; field += 16)
        fill_car_history(offset(car, field), &zero);
    construct_control_state(offset(car, 1620));
    car_words[0] = reinterpret_cast<Dword>(car_base_methods);
    for (unsigned word = 442; word <= 450; ++word)
        car_words[word] = 0;
    at<Byte>(car, 1816) = 0;
    construct_car_run_statistics(offset(car, 1820));
    bool queue_ready = false;
    try {
        for (unsigned wheel = 0; wheel < 4; ++wheel) {
            void *state = offset(car, 1948 + wheel * 296);
            runtime::physics::pending::construct_ground_contact(offset(state, 48));
            initialize_wheel_state(state, nullptr, nullptr, 0, nullptr);
        }
        car_words[783] = car_words[802] = car_words[843] = 0;
        InitializeCriticalSection(&network::snapshots::car_queue(car)->critical);
        network::snapshots::reset_queue(offset(car, 3376));
        queue_ready = true;
        construct_impact_state(offset(car, 7952), car);
        car_words[1996] = 0;
        car_words[1997] = 0;
        car_words[1998] = 0;
        car_words[1999] = 0;
        car_words[2000] = 0;
        *car_words = reinterpret_cast<Dword>(car_position_methods);
        car_words[488] = reinterpret_cast<Dword>(car_words + 228);
        car_words[487] = reinterpret_cast<Dword>(car_words);
        car_words[490] = 0;
        car_words[489] = reinterpret_cast<Dword>(car_words + 366);
        car_words[491] = 0;
        car_words[492] = 0;
        car_words[493] = 0;
        car_words[494] = 0;
        car_words[495] = 0;
        car_words[496] = 0;
        car_words[497] = 0;
        car_words[498] = 0;
        kernel::memory_zero(car_words + 499, 84);
        car_words[520] = 1;
        kernel::memory_zero(car_words + 521, 8);
        car_words[523] = 0;
        car_words[524] = 0;
        car_words[525] = 0;
        car_words[526] = 0;
        car_words[527] = 0;
        car_words[528] = 0;
        car_words[529] = 0;
        car_words[530] = 0;
        car_words[531] = 0;
        car_words[532] = 0;
        car_words[533] = 0;
        car_words[534] = 0;
        car_words[535] = 0;
        car_words[536] = 0;
        car_words[537] = 0;
        car_words[538] = 0;
        car_words[539] = 0;
        car_words[540] = 0;
        car_words[541] = 0;
        *reinterpret_cast<Byte *>(car_words + 542) = 0;
        *reinterpret_cast<Byte *>(reinterpret_cast<Dword>(car_words) + 2169) = 0;
        car_words[543] = 0;
        car_words[544] = 0;
        car_words[545] = 0;
        car_words[546] = 0;
        car_words[547] = 0;
        car_words[548] = 0;
        car_words[549] = 0;
        car_words[550] = 0;
        *reinterpret_cast<Byte *>(car_words + 551) = 0;
        car_words[552] = 0;
        car_words[553] = 0;
        car_words[554] = 0;
        car_words[555] = 0;
        car_words[559] = 0;
        car_words[560] = 0;
        car_words[556] = 0;
        car_words[557] = 0;
        car_words[558] = 0;
        car_words[562] = reinterpret_cast<Dword>(car_words + 228);
        car_words[561] = reinterpret_cast<Dword>(car_words);
        car_words[564] = 1;
        car_words[563] = reinterpret_cast<Dword>(car_words + 366);
        car_words[565] = 0;
        car_words[566] = 0;
        car_words[567] = 0;
        car_words[568] = 0;
        car_words[569] = 0;
        car_words[570] = 0;
        car_words[571] = 0;
        car_words[572] = 0;
        kernel::memory_zero(car_words + 573, 84);
        car_words[594] = 1;
        kernel::memory_zero(car_words + 595, 8);
        car_words[597] = 0;
        car_words[598] = 0;
        car_words[599] = 0;
        car_words[600] = 0;
        car_words[601] = 0;
        car_words[602] = 0;
        car_words[603] = 0;
        car_words[604] = 0;
        car_words[605] = 0;
        car_words[606] = 0;
        car_words[607] = 0;
        car_words[608] = 0;
        car_words[609] = 0;
        car_words[610] = 0;
        car_words[611] = 0;
        car_words[612] = 0;
        car_words[613] = 0;
        car_words[614] = 0;
        car_words[615] = 0;
        *reinterpret_cast<Byte *>(car_words + 616) = 0;
        *reinterpret_cast<Byte *>(reinterpret_cast<Dword>(car_words) + 2465) = 0;
        car_words[617] = 0;
        car_words[618] = 0;
        car_words[619] = 0;
        car_words[620] = 0;
        car_words[621] = 0;
        car_words[622] = 0;
        car_words[623] = 0;
        car_words[630] = 0;
        car_words[631] = 0;
        car_words[632] = 0;
        car_words[624] = 0;
        *reinterpret_cast<Byte *>(car_words + 625) = 0;
        car_words[626] = 0;
        car_words[627] = 0;
        car_words[628] = 0;
        car_words[629] = 0;
        car_words[633] = 0;
        car_words[634] = 0;
        initialize_wheel_state(car_words + 635, car_words + 297, car_words + 370, 2,
                               car_words);
        initialize_wheel_state(car_words + 709, car_words + 297, car_words + 370, 3,
                               car_words);
        car_words[451] = 0;
        car_words[452] = 0;
        car_words[453] = 0;
        car_words[784] = 0;
        car_words[785] = 0;
        car_words[786] = 0;
        car_words[787] = 0;
        car_words[788] = 0;
        car_words[789] = 0;
        car_words[790] = 0;
        car_words[791] = 0;
        car_words[792] = 0;
        car_words[793] = 0;
        car_words[794] = 0;
        car_words[795] = 0;
        car_words[796] = 0;
        car_words[797] = 0;
        car_words[798] = 0;
        car_words[799] = 0;
        car_words[800] = 0;
        car_words[393] = 0;
        car_words[392] = 0;
        car_words[391] = 0;
        car_words[801] = 0;

    } catch (...) {
        if (queue_ready)
            network::snapshots::destroy_queue(offset(car, 3376));
        application::pending::destroy_car_run_statistics(
            reinterpret_cast<application::pending::CarRunStatistics *>(offset(car, 1820)));
        throw;
    }
    return car;
}
} // namespace nfs::game::world::pending
