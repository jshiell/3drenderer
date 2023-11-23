#include "clipping.h"

#include <math.h>

#define NUM_PLANES 6
plane_t frustum_planes[NUM_PLANES];

void initialise_frustum_planes(float fov, float z_near, float z_far) {
    float cos_half_fov = cos(fov / 2.0f);
    float sin_half_fov = sin(fov / 2.0f);

    frustum_planes[LEFT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov;

    frustum_planes[RIGHT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov;

    frustum_planes[TOP_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov;
    frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov;

    frustum_planes[BOTTOM_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov;
    frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov;

    frustum_planes[NEAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_near);
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

    frustum_planes[FAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_far);
    frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
    frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}
