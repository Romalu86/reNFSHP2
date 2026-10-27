#include "camera_director_runtime.hpp"
namespace nfs::game::world::camera {

const Dword actor_transform_methods[] = {reinterpret_cast<Dword>(delete_actor_transform),
                                        reinterpret_cast<Dword>(advance_actor_transform),
                                        reinterpret_cast<Dword>(begin_actor_transform)};

const Dword actor_clip_methods[] = {reinterpret_cast<Dword>(delete_actor_clip),
                                   reinterpret_cast<Dword>(advance_actor_clip),
                                   reinterpret_cast<Dword>(end_position_behavior)};

const Dword follow_fov_methods[] = {reinterpret_cast<Dword>(delete_follow_fov),
                                   reinterpret_cast<Dword>(advance_follow_fov),
                                   reinterpret_cast<Dword>(begin_follow_fov)};

const Dword follow_look_methods[] = {reinterpret_cast<Dword>(delete_follow_look),
                                    reinterpret_cast<Dword>(advance_follow_look),
                                    reinterpret_cast<Dword>(begin_follow_look)};

const Dword mount_look_methods[] = {reinterpret_cast<Dword>(delete_mount_look),
                                   reinterpret_cast<Dword>(advance_mount_look),
                                   reinterpret_cast<Dword>(end_position_behavior)};

const Dword point_look_methods[] = {reinterpret_cast<Dword>(delete_point_look),
                                   reinterpret_cast<Dword>(advance_point_look),
                                   reinterpret_cast<Dword>(end_position_behavior)};

const Dword follow_position_methods[] = {reinterpret_cast<Dword>(delete_follow_position),
                                        reinterpret_cast<Dword>(advance_follow_position),
                                        reinterpret_cast<Dword>(begin_follow_position)};

const Dword matrix_position_methods[] = {reinterpret_cast<Dword>(delete_matrix_position),
                                        reinterpret_cast<Dword>(advance_matrix_position),
                                        reinterpret_cast<Dword>(end_position_behavior)};

const Dword mount_position_methods[] = {reinterpret_cast<Dword>(delete_mount_position),
                                       reinterpret_cast<Dword>(advance_mount_position),
                                       reinterpret_cast<Dword>(begin_mount_position)};

const Dword pause_position_methods[] = {reinterpret_cast<Dword>(delete_pause_position),
                                       reinterpret_cast<Dword>(advance_pause_position),
                                       reinterpret_cast<Dword>(end_position_behavior)};

const Dword follow_up_methods[] = {reinterpret_cast<Dword>(delete_follow_up),
                                  reinterpret_cast<Dword>(advance_follow_up),
                                  reinterpret_cast<Dword>(end_position_behavior)};

const Dword matrix_controller_methods[] = {reinterpret_cast<Dword>(begin_matrix_controller),
                                          reinterpret_cast<Dword>(end_matrix_controller),
                                          reinterpret_cast<Dword>(advance_matrix_controller)};
} // namespace nfs::game::world::camera
