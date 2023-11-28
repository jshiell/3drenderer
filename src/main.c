#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "upng.h"
#include "array.h"
#include "camera.h"
#include "clipping.h"
#include "display.h"
#include "light.h"
#include "matrix.h"
#include "mesh.h"
#include "texture.h"
#include "triangle.h"
#include "vector.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;

mat4_t proj_matrix;

void setup(void) {
    set_render_method(RENDER_TEXTURED);
    set_cull_method(CULL_BACKFACE);

    init_light(vec3_new(0, 0, 1));

    float aspect_ratio_y = (float) get_window_height() / (float) get_window_width();
    float fov_y = M_PI / 3.0f; // 60 degrees
    float aspect_ratio_x = (float) get_window_width() / (float) get_window_height();
    float fov_x = atan(tan(fov_y / 2.0f) * aspect_ratio_x) * 2;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov_y, aspect_ratio_y, znear, zfar);

    initialise_frustum_planes(fov_x, fov_y, znear, zfar);

    load_mesh("assets/runway.obj", "assets/runway.png", vec3_new(1, 1, 1), vec3_new(0, -1.5, 23), vec3_new(0, 0, 0));
    load_mesh("assets/f22.obj", "assets/f22.png", vec3_new(1, 1, 1), vec3_new(0, -1.3, 5), vec3_new(0, -M_PI/2, 0));
    load_mesh("assets/efa.obj", "assets/efa.png", vec3_new(1, 1, 1), vec3_new(-2, -1.3, 9), vec3_new(0, -M_PI/2, 0));
    load_mesh("assets/f117.obj", "assets/f117.png", vec3_new(1, 1, 1), vec3_new(2, -1.3, 9), vec3_new(0, -M_PI/2, 0));
}

void process_input(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                is_running = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        rotate_camera_pitch(15.0 * delta_time);
                        break;
                    case SDLK_s:
                        rotate_camera_pitch(-15.0 * delta_time);
                        break;
                    case SDLK_RIGHT:
                        rotate_camera_yaw(5.0 * delta_time);
                        break;
                    case SDLK_LEFT:
                        rotate_camera_yaw(-5.0 * delta_time);
                        break;
                    case SDLK_UP:
                        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 15.0 * delta_time));
                        update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
                        break;
                    case SDLK_DOWN:
                        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 15.0 * delta_time));
                        update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
                        break;
                    case SDLK_ESCAPE:
                        is_running = false;
                        break;
                    case SDLK_c:
                        set_cull_method(CULL_BACKFACE);
                        break;
                    case SDLK_x:
                        set_cull_method(CULL_NONE);
                        break;
                    case SDLK_1:
                        set_render_method(RENDER_WIRE_VERTEX);
                        break;
                    case SDLK_2:
                        set_render_method(RENDER_WIRE);
                        break;
                    case SDLK_3:
                        set_render_method(RENDER_FILL_TRIANGLE);
                        break;
                    case SDLK_4:
                        set_render_method(RENDER_FILL_TRIANGLE_WIRE);
                        break;
                    case SDLK_5:
                        set_render_method(RENDER_TEXTURED);
                        break;
                    case SDLK_6:
                        set_render_method(RENDER_TEXTURED_WIRE);
                        break;
                }
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline stages for all the mesh triangles
///////////////////////////////////////////////////////////////////////////////
// +-------------+
// | Model space |  <-- original mesh vertices
// +-------------+
// |   +-------------+
// `-> | World space |  <-- multiply by world matrix
//     +-------------+
//     |   +--------------+
//     `-> | Camera space |  <-- multiply by view matrix
//         +--------------+
//         |    +------------+
//         `--> |  Clipping  |  <-- clip against the six frustum planes
//              +------------+
//              |    +------------+
//              `--> | Projection |  <-- multiply by projection matrix
//                   +------------+
//                   |    +-------------+
//                   `--> | Image space |  <-- apply perspective divide
//                        +-------------+
//                        |    +--------------+
//                        `--> | Screen space |  <-- ready to render
//                             +--------------+
///////////////////////////////////////////////////////////////////////////////
void process_graphics_pipeline_stages(mesh_t* mesh) {
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

    vec3_t up_direction = { 0, 1, 0 };
    vec3_t target = get_camera_lookat_target();
    mat4_t view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

    // order matters: scale -> rotate -> translate
    mat4_t world_matrix = mat4_identity();
    world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
    world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

    for (int i = 0; i < array_length(mesh->faces); ++i) {
        face_t mesh_face = mesh->faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

        // transform
        vec4_t transformed_vertices[3];
        for (int j = 0; j < 3; ++j) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // transform to view space
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            transformed_vertices[j] = transformed_vertex;
        }

        vec3_t face_normal = get_triangle_normal(transformed_vertices);

        if (is_cull_backface()) {
            // find vector from triangle to the origin
            vec3_t camera_ray = vec3_sub(vec3_new(0, 0, 0), vec3_from_vec4(transformed_vertices[0]));

            float dot_normal_camera = vec3_dot(face_normal, camera_ray);
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        // clipping
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv
        );
        clip_polygon(&polygon);

        // break polygon back into triangles
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;
        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        for (int t = 0; t < num_triangles_after_clipping; ++t) {
            triangle_t triangle_after_clipping = triangles_after_clipping[t];
            
            // project into screen space
            vec4_t projected_points[3];
            for (int j = 0; j < 3; ++j) {
                projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

                // scale to viewport
                projected_points[j].x *= (get_window_width() / 2.0);
                projected_points[j].y *= (get_window_height() / 2.0);

                // invert y values to account for flipped y axis
                projected_points[j].y *= -1;

                // translate to middle of screen
                projected_points[j].x += (get_window_width() / 2.0);
                projected_points[j].y += (get_window_height() / 2.0);
            }

            // flat shading, base intensity on alignment with inverse of the light direction
            float light_intensity_factor = -vec3_dot(face_normal, get_light_direction());
            uint32_t triangle_colour = light_apply_intensity(mesh_face.colour, light_intensity_factor);

            triangle_t triangle_to_render = {
                .points = {
                    { projected_points[0].x , projected_points[0].y, projected_points[0].z, projected_points[0].w },
                    { projected_points[1].x , projected_points[1].y, projected_points[1].z, projected_points[1].w },
                    { projected_points[2].x , projected_points[2].y, projected_points[2].z, projected_points[2].w }
                },
                .texcoords = {
                    { triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
                    { triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
                    { triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
                },
                .colour = triangle_colour,
                .texture = mesh->texture
            };

            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
                triangles_to_render[num_triangles_to_render++] = triangle_to_render;
            }
        }
    }
}

void update(void) {
    int time_to_wait =  FRAME_TARGET_TIME - SDL_GetTicks() - previous_frame_time;
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;
    previous_frame_time = SDL_GetTicks();

    num_triangles_to_render = 0;

    for (int mesh_index = 0; mesh_index < get_num_meshes(); ++mesh_index) {
        mesh_t* mesh = get_mesh(mesh_index);

        // mesh->rotation.x += 0.05 * delta_time;
        // mesh->rotation.y += 0.5 * delta_time;
        // mesh->rotation.z += 0.01 * delta_time;
        // mesh->scale.x += 0.002 * delta_time;
        // mesh->scale.y += 0.001 * delta_time;
        // mesh->translation.x += 0.01 * delta_time;
        // mesh->translation.z = 5.0;

        process_graphics_pipeline_stages(mesh);
    }
}

void render(void) {
    clear_colour_buffer(0xFF000000);
    clear_z_buffer();

    draw_grid();

    for (int i = 0; i < num_triangles_to_render; ++i) {
        triangle_t triangle = triangles_to_render[i];

        if (should_render_filled_triangles()) {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
                triangle.colour);
        }

        if (should_render_textured_triangles()) {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
                triangle.texture);
        }

        if (should_render_wireframe()) {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFFFF);
        }

        if (should_render_wire_vertex()) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }
    }

    render_colour_buffer();
}

void free_resources(void) {
    free_meshes();
    destroy_window();
}

int main(void) {
    is_running = initialise_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    free_resources();

    return 0;
}
