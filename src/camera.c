#include "camera.h"
#include "matrix.h"

static camera_t camera = {
    .position = { 0, 0, 0 },
    .direction = { 0, 0, 1 },
    .forward_velocity = { 0, 0, 0 },
    .yaw = 0,
    .pitch = 0
};

void init_camera(vec3_t position, vec3_t direction) {
    camera.position = position;
    camera.direction = direction;
    camera.forward_velocity = vec3_new(0, 0, 0);
    camera.yaw = 0.0;
    camera.pitch = 0.0;
}

vec3_t get_camera_position(void) {
    return camera.position;
}

void update_camera_position(vec3_t position) {
    camera.position = position;
}

vec3_t get_camera_direction(void) {
    return camera.direction;
}

void update_camera_direction(vec3_t direction) {
    camera.direction = direction;
}

vec3_t get_camera_forward_velocity(void) {
    return camera.forward_velocity;
}

void update_camera_forward_velocity(vec3_t forward_velocity) {
    camera.forward_velocity = forward_velocity;
}

float get_camera_yaw(void) {
    return camera.yaw;
}

void rotate_camera_yaw(float angle) {
    camera.yaw += angle;
}

float get_camera_pitch(void) {
    return camera.pitch;
}

void rotate_camera_pitch(float angle) {
    camera.pitch += angle;
}

vec3_t get_camera_lookat_target(void) {
    vec3_t target = { 0, 0, 1 };

    mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
    mat4_t camera_pitch_rotation = mat4_make_rotation_x(camera.pitch);

    mat4_t camera_rotation = mat4_identity();
    camera_rotation = mat4_mul_mat4(camera_pitch_rotation, camera_rotation);
    camera_rotation = mat4_mul_mat4(camera_yaw_rotation, camera_rotation);

    vec4_t camera_direction = mat4_mul_vec4(camera_rotation, vec4_from_vec3(target));
    camera.direction = vec3_from_vec4(camera_direction);

    target = vec3_add(camera.position, camera.direction);
    return target;
}
