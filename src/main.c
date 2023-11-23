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

mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

void setup(void) {
    render_method = RENDER_TEXTURED;
    cull_method = CULL_BACKFACE;

    colour_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
    z_buffer = (float*) malloc(sizeof(float) * window_width * window_height);

    colour_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    float fov = M_PI / 3.0f; // 60 degrees
    float aspect_ratio = (float) window_height / (float) window_width;
    float znear = 0.1;
    float zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect_ratio, znear, zfar);

    initialise_frustum_planes(fov, znear, zfar);

    // load_cube_mesh_data();
    char* filename = "assets/f22.obj";
    if (!load_obj_file_data(filename)) {
        fprintf(stderr, "Failed to load obj file data from %s\n", filename);
        exit(1);
    }
    load_png_texture_data("assets/f22.png");
}

void process_input(void) {
    SDL_Event event;

    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_w:
                    camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
                    camera.position = vec3_add(camera.position, camera.forward_velocity);
                    break;
                case SDLK_s:
                    camera.forward_velocity = vec3_mul(camera.direction, 5.0 * delta_time);
                    camera.position = vec3_sub(camera.position, camera.forward_velocity);
                    break;
                case SDLK_a:
                    camera.yaw += 1.0 * delta_time;
                    break;
                case SDLK_d:
                    camera.yaw -= 1.0 * delta_time;
                    break;
                case SDLK_UP:
                    camera.position.y += 3.0 * delta_time;
                    break;
                case SDLK_DOWN:
                    camera.position.y -= 3.0 * delta_time;
                    break;
                case SDLK_ESCAPE:
                    is_running = false;
                    break;
                case SDLK_c:
                    cull_method = CULL_BACKFACE;
                    break;
                case SDLK_x:
                    cull_method = CULL_NONE;
                    break;
                case SDLK_1:
                    render_method = RENDER_WIRE_VERTEX;
                    break;
                case SDLK_2:
                    render_method = RENDER_WIRE;
                    break;
                case SDLK_3:
                    render_method = RENDER_FILL_TRIANGLE;
                    break;
                case SDLK_4:
                    render_method = RENDER_FILL_TRIANGLE_WIRE;
                    break;
                case SDLK_5:
                    render_method = RENDER_TEXTURED;
                    break;
                case SDLK_6:
                    render_method = RENDER_TEXTURED_WIRE;
                    break;
            }
            break;
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

    // mesh.rotation.x += 0.05;
    // mesh.rotation.y += 0.5 * delta_time;
    // mesh.rotation.z += 0.01 * delta_time;
    // mesh.scale.x += 0.002 * delta_time;
    // mesh.scale.y += 0.001 * delta_time;
    // mesh.translation.x += 0.01 * delta_time;
    mesh.translation.z = 5.0;

    vec3_t up_direction = { 0, 1, 0 };
    vec3_t target = { 0, 0, 1 };
    mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
    camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));
    // offset camera
    target = vec3_add(camera.position, camera.direction);

    view_matrix = mat4_look_at(camera.position, target, up_direction);

    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    world_matrix = mat4_identity();
    world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
    world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
    world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

    for (int i = 0; i < array_length(mesh.faces); ++i) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a];
        face_vertices[1] = mesh.vertices[mesh_face.b];
        face_vertices[2] = mesh.vertices[mesh_face.c];

        // transform
        vec4_t transformed_vertices[3];
        for (int j = 0; j < 3; ++j) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // transform to view space
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            transformed_vertices[j] = transformed_vertex;
        }

        // back-face culling
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A  */  
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \ */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* C--B */

        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalise(&vector_ab);
        vec3_normalise(&vector_ac);

        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalise(&normal);

        if (cull_method == CULL_BACKFACE) {
            // find vector from triangle to the origin
            vec3_t origin = { 0, 0, 0 };
            vec3_t camera_ray = vec3_sub(origin, vector_a);

            float dot_normal_camera = vec3_dot(normal, camera_ray);
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        // project
        vec4_t projected_points[3];
        for (int j = 0; j < 3; ++j) {
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // scale to viewport
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            // invert y values to account for flipped y axis
            projected_points[j].y *= -1;

            // translate to middle of screen
            projected_points[j].x += (window_width / 2.0);
            projected_points[j].y += (window_height / 2.0);
        }

        // flat shading, base intensity on alignment with inverse of the light direction
        float light_intensity_factor = -vec3_dot(normal, light.direction);
        uint32_t triangle_colour = light_apply_intensity(mesh_face.colour, light_intensity_factor);

        triangle_t projected_triangle = {
            .points = {
                { projected_points[0].x , projected_points[0].y, projected_points[0].z, projected_points[0].w },
                { projected_points[1].x , projected_points[1].y, projected_points[1].z, projected_points[1].w },
                { projected_points[2].x , projected_points[2].y, projected_points[2].z, projected_points[2].w }
            },
            .texcoords = {
                { mesh_face.a_uv.u, mesh_face.a_uv.v },
                { mesh_face.b_uv.u, mesh_face.b_uv.v },
                { mesh_face.c_uv.u, mesh_face.c_uv.v }
            },
            .colour = triangle_colour
        };

        if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH) {
            triangles_to_render[num_triangles_to_render++] = projected_triangle;
        }
    }
}

void render(void) {
    SDL_RenderClear(renderer);

    draw_grid();

    for (int i = 0; i < num_triangles_to_render; ++i) {
        triangle_t triangle = triangles_to_render[i];

        if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE) {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
                triangle.colour);
        }

        if (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE) {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
                mesh_texture);
        }

        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX
                || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRE) {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFFFF);
        }

        if (render_method == RENDER_WIRE_VERTEX) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }
    }

    render_colour_buffer();
    clear_colour_buffer(0xFF000000);
    clear_z_buffer();

    SDL_RenderPresent(renderer);
}

void free_resources(void) {
    free(z_buffer);
    free(colour_buffer);
    upng_free(png_texture);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

int main(void) {
    is_running = initialise_window();

    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
