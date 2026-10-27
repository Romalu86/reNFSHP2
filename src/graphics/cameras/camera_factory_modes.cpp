#include "camera_factory_support.hpp"
namespace nfs::game::world::camera {

void create_bumper_camera(void *director) {
    float bumper_fov = 90.0f;
    float bumper_arm_height = 0.37999999523162841796875f;
    float bumper_pitch = 0.4180000126361846923828125f;
    float bumper_2plr_fov = 90.0f;
    float bumper_2plr_arm_height = 0.37999999523162841796875f;
    float bumper_2plr_pitch = 0.4180000126361846923828125f;
    float bumper_2ptb_fov = 90.0f;
    float bumper_2ptb_arm_height = 0.37999999523162841796875f;
    float bumper_2ptb_pitch = 0.4180000126361846923828125f;
    read_camera_configuration([&](ini::Reader *reader) {
        ini::get_float(reader, "bumper", "FOV", &bumper_fov);
        ini::get_float(reader, "bumper", "ArmHeight", &bumper_arm_height);
        ini::get_float(reader, "bumper", "Pitch", &bumper_pitch);
        ini::get_float(reader, "bumper-2plr", "FOV", &bumper_2plr_fov);
        ini::get_float(reader, "bumper-2plr", "ArmHeight", &bumper_2plr_arm_height);
        ini::get_float(reader, "bumper-2plr", "Pitch", &bumper_2plr_pitch);
        ini::get_float(reader, "bumper-2ptb", "FOV", &bumper_2ptb_fov);
        ini::get_float(reader, "bumper-2ptb", "ArmHeight", &bumper_2ptb_arm_height);
        ini::get_float(reader, "bumper-2ptb", "Pitch", &bumper_2ptb_pitch);
    });
    void *controller = allocate_controller(director, 3);
    void *position =
        allocate_behavior(120, "behavior - position mount on car", construct_mount_position);
    at<float>(position, 20) = bumper_arm_height;
    save_full_mount_height(position);
    at<float>(position, 20) = bumper_2plr_arm_height;
    save_vertical_mount_height(position);
    at<float>(position, 20) = bumper_2ptb_arm_height;
    save_horizontal_mount_height(position);
    at<void *>(controller, 80) = position;
    void *look = allocate_behavior(36, "behavior - look at mount on car", construct_mount_look);
    at<Dword>(look, 8) = at<Dword>(look, 12) = 0;
    at<float>(look, 16) = 1;
    at<float>(look, 20) = bumper_pitch;
    save_full_mount_pitch(look);
    at<float>(look, 20) = bumper_2plr_pitch;
    save_vertical_mount_pitch(look);
    at<float>(look, 20) = bumper_2ptb_pitch;
    save_horizontal_mount_pitch(look);
    at<void *>(controller, 84) = look;
    at<float>(controller, 36) = bumper_fov;
    save_full_interval(offset(controller, 4));
    at<float>(controller, 36) = bumper_2plr_fov;
    save_vertical_interval(offset(controller, 4));
    at<float>(controller, 36) = bumper_2ptb_fov;
    save_horizontal_interval(offset(controller, 4));
    append_camera_mode(director, controller);
}

void create_matrix_camera(void *director) {
    float matrix_matrix_speed = 0.100000001490116119384765625f;
    float matrix_matrix_fov = 60.0f;
    float matrix_matrix_arm_vector_x = 4.5f;
    float matrix_matrix_arm_vector_y = 0.0f;
    float matrix_matrix_arm_vector_z = 7.88000011444091796875f;
    read_camera_configuration([&](ini::Reader *reader) {
        ini::get_float(reader, "matrix", "MatrixSpeed", &matrix_matrix_speed);
        ini::get_float(reader, "matrix", "MatrixFOV", &matrix_matrix_fov);
        ini::get_float(reader, "matrix", "MatrixArmVectorX", &matrix_matrix_arm_vector_x);
        ini::get_float(reader, "matrix", "MatrixArmVectorY", &matrix_matrix_arm_vector_y);
        ini::get_float(reader, "matrix", "MatrixArmVectorZ", &matrix_matrix_arm_vector_z);
    });
    void *memory = lifecycle::allocate(108, "camera controller", lifecycle::level_pool);
    void *controller = memory ? construct_controller(memory) : nullptr;
    if (controller)
        at<const Dword *>(controller, 0) = matrix_controller_methods;
    at<Dword>(controller, 76) = at<Dword>(controller, 104) = 9;
    at<void *>(controller, 4) = director;
    for (unsigned axis = 0; axis < 3; ++axis)
        at<float>(controller, 64 + axis * 4) = up_direction[axis];
    void *position =
        allocate_behavior(120, "behavior - position matrix", construct_matrix_position);
    at<float>(position, 20) = matrix_matrix_arm_vector_x;
    at<float>(position, 24) = matrix_matrix_arm_vector_y;
    at<float>(position, 28) = matrix_matrix_arm_vector_z;
    at<float>(position, 12) = matrix_matrix_speed;
    at<float>(position, 20) = matrix_matrix_arm_vector_x;
    at<float>(position, 24) = matrix_matrix_arm_vector_y;
    at<float>(position, 28) = matrix_matrix_arm_vector_z;
    at<void *>(controller, 80) = position;
    at<void *>(controller, 84) =
        allocate_behavior(8, "behavior - look at point", construct_point_look);
    at<float>(controller, 36) = at<float>(controller, 32) = matrix_matrix_fov;
    append_camera_mode(director, controller);
}

void create_jump_camera(void *director) {
    float jump_jump_threshold = 16.0f;
    float jump_jump_minimum_hold = 0.75f;
    float jump_jump_maximum_hold = 2.75f;
    float jump_jump_clock_speed1 = 0.5f;
    float jump_jump_fov1 = 45.0f;
    float jump_jump_amount_lead_car1 = 5.0f;
    float jump_jump_camera_height1 = 3.0f;
    float jump_jump_clock_speed2 = 0.5f;
    float jump_jump_fov2 = 45.0f;
    float jump_jump_amount_lead_car2 = 5.0f;
    float jump_jump_camera_height2 = 3.0f;
    float jump_jump_clock_speed3 = 0.5f;
    float jump_jump_fov3 = 45.0f;
    float jump_jump_amount_lead_car3 = 5.0f;
    float jump_jump_camera_height3 = 3.0f;
    read_camera_configuration([&](ini::Reader *reader) {
        ini::get_float(reader, "jump", "JumpThreshold", &jump_jump_threshold);
        ini::get_float(reader, "jump", "JumpMinimumHold", &jump_jump_minimum_hold);
        ini::get_float(reader, "jump", "JumpMaximumHold", &jump_jump_maximum_hold);
        ini::get_float(reader, "jump", "JumpClockSpeed1", &jump_jump_clock_speed1);
        ini::get_float(reader, "jump", "JumpFOV1", &jump_jump_fov1);
        ini::get_float(reader, "jump", "JumpAmountLeadCar1", &jump_jump_amount_lead_car1);
        ini::get_float(reader, "jump", "JumpCameraHeight1", &jump_jump_camera_height1);
        ini::get_float(reader, "jump", "JumpClockSpeed2", &jump_jump_clock_speed2);
        ini::get_float(reader, "jump", "JumpFOV2", &jump_jump_fov2);
        ini::get_float(reader, "jump", "JumpAmountLeadCar2", &jump_jump_amount_lead_car2);
        ini::get_float(reader, "jump", "JumpCameraHeight2", &jump_jump_camera_height2);
        ini::get_float(reader, "jump", "JumpClockSpeed3", &jump_jump_clock_speed3);
        ini::get_float(reader, "jump", "JumpFOV3", &jump_jump_fov3);
        ini::get_float(reader, "jump", "JumpAmountLeadCar3", &jump_jump_amount_lead_car3);
        ini::get_float(reader, "jump", "JumpCameraHeight3", &jump_jump_camera_height3);
    });
    at<float>(director, 184) = jump_jump_threshold;
    at<float>(director, 196) = jump_jump_clock_speed1;
    at<float>(director, 204) = jump_jump_camera_height1;
    at<float>(director, 188) = jump_jump_minimum_hold;
    at<float>(director, 192) = jump_jump_maximum_hold;
    at<float>(director, 224) = jump_jump_amount_lead_car2;
    at<float>(director, 200) = jump_jump_fov1;
    at<float>(director, 208) = jump_jump_amount_lead_car1;
    at<float>(director, 232) = jump_jump_fov3;
    at<float>(director, 212) = jump_jump_clock_speed2;
    at<float>(director, 216) = jump_jump_fov2;
    at<float>(director, 220) = jump_jump_camera_height2;
    at<float>(director, 228) = jump_jump_clock_speed3;
    at<float>(director, 240) = jump_jump_amount_lead_car3;
    at<float>(director, 236) = jump_jump_camera_height3;
    void *controller = allocate_controller(director, 4);
    at<void *>(controller, 84) =
        allocate_behavior(8, "behavior - look at point", construct_point_look);
    at<float>(controller, 36) = at<float>(controller, 32) = jump_jump_fov1;
    append_camera_mode(director, controller);
}

void read_master_configuration(void *director) {
    int master_jump_camera = 2;
    int master_acceleration_enhancement = 1;
    int master_speed_enhancement = 1;
    int master_look_back_target = 1;
    read_camera_configuration([&](ini::Reader *reader) {
        ini::get_int(reader, "master", "JumpCamera", &master_jump_camera);
        ini::get_int(reader, "master", "AccelerationEnhancement", &master_acceleration_enhancement);
        ini::get_int(reader, "master", "SpeedEnhancement", &master_speed_enhancement);
        ini::get_int(reader, "master", "LookBackTarget", &master_look_back_target);
    });
    at<Dword>(director, 168) = master_jump_camera == 0 ? 0 : master_jump_camera == 1 ? 1 : 2;
    void *high = find_camera_mode(director, 1);
    void *low = find_camera_mode(director, 0);
    void *bird = find_camera_mode(director, 2);
    if (high && low && bird) {
        void *high_position = at<void *>(high, 80);
        void *low_position = at<void *>(low, 80);
        void *bird_position = at<void *>(bird, 80);
        if (high_position) {
            at<Byte>(high_position, 97) = master_acceleration_enhancement != 0;
            at<Byte>(low_position, 97) = master_acceleration_enhancement != 0;
            at<Byte>(bird_position, 97) = master_acceleration_enhancement != 0;
        }
        if (bird) {
            at<Byte>(at<void *>(high, 88), 32) = master_speed_enhancement != 0;
            at<Byte>(at<void *>(low, 88), 32) = master_speed_enhancement != 0;
            at<Byte>(at<void *>(bird, 88), 32) = master_speed_enhancement != 0;
        }
    }
    at<Byte>(director, 167) = master_look_back_target != 0;
}

void create_pause_camera(void *director) {
    void *controller = allocate_controller(director, 8);
    void *position = allocate_behavior(112, "behavior - position pause", construct_pause_position);
    at<float>(position, 12) = 4.5f;
    at<Dword>(position, 16) = 0;
    at<float>(position, 20) = 7.88000011444091796875f;
    at<void *>(controller, 80) = position;
    at<void *>(controller, 84) =
        allocate_behavior(8, "behavior - look at point", construct_point_look);
    at<float>(controller, 36) = at<float>(controller, 32) = 50;
    append_camera_mode(director, controller);
}

void create_actor_camera(void *director) {
    void *controller = allocate_controller(director, 6, false);
    at<void *>(controller, 92) =
        allocate_behavior(8, "behavior - Actor Clip", construct_actor_clip);
    at<void *>(controller, 100) =
        allocate_behavior(8, "behavior - Actor Transformation", construct_actor_transform);
    append_camera_mode(director, controller);
}
} // namespace nfs::game::world::camera
