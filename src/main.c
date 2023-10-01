#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "mesh.h"
#include "vector.h"

triangle_t* triangles_to_render = NULL;

bool is_running = false;
int previous_frame_time = 0;

vec3_t camera_position = { .x = 0, .y = 0, .z = 0 };
float fov_factor = 640;

enum render_mode_t {
    RENDER_MODE_WIREFRAME_POINTS,
    RENDER_MODE_WIREFRAME,
    RENDER_MODE_WIREFRAME_FILL,
    RENDER_MODE_FILL,
};
enum render_mode_t render_mode = RENDER_MODE_WIREFRAME_POINTS;
bool back_face_culling = true;

void setup(void) {
    colour_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    colour_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    char* filename = "assets/cube.obj";
    if (!load_obj_file_data(filename)) {
        fprintf(stderr, "Failed to load obj file data from %s\n", filename);
        exit(1);
    }
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
                case SDLK_ESCAPE:
                    is_running = false;
                    break;
                case SDLK_c:
                    back_face_culling = !back_face_culling;
                    break;
                case SDLK_1:
                    render_mode = RENDER_MODE_WIREFRAME_POINTS;
                    break;
                case SDLK_2:
                    render_mode = RENDER_MODE_WIREFRAME;
                    break;
                case SDLK_3:
                    render_mode = RENDER_MODE_FILL;
                    break;
                case SDLK_4:
                    render_mode = RENDER_MODE_WIREFRAME_FILL;
                    break;
            }
            break;
    }
}

vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
    };
    return projected_point;
}

void update(void) {
    int time_to_wait =  FRAME_TARGET_TIME - SDL_GetTicks() - previous_frame_time;
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    previous_frame_time = SDL_GetTicks();

    triangles_to_render = NULL;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    for (int i = 0; i < array_length(mesh.faces); ++i) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        // transform
        vec3_t transformed_vertices[3];

        for (int j = 0; j < 3; ++j) {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // translate away from camera
            transformed_vertex.z += 5;

            transformed_vertices[j] = transformed_vertex;
        }

        // back-face culling
        if (back_face_culling) {
            vec3_t vector_a = transformed_vertices[0]; /*   A  */  
            vec3_t vector_b = transformed_vertices[1]; /*  / \ */
            vec3_t vector_c = transformed_vertices[2]; /* C--B */

            vec3_t vector_ab = vec3_sub(vector_b, vector_a);
            vec3_t vector_ac = vec3_sub(vector_c, vector_a);
            vec3_normalise(&vector_ab);
            vec3_normalise(&vector_ac);

            vec3_t normal = vec3_cross(vector_ab, vector_ac);
            vec3_normalise(&normal);

            // find vector from triangle to the camera
            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            float dot_normal_camera = vec3_dot(normal, camera_ray);
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        // project
        triangle_t projected_triangle;

        for (int j = 0; j < 3; ++j) {
            vec2_t projected_point = project(transformed_vertices[j]);

            // translate to middle of screen
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }

        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void) {
    draw_grid();

    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; ++i) {
        triangle_t triangle = triangles_to_render[i];

        if (render_mode == RENDER_MODE_FILL || render_mode == RENDER_MODE_WIREFRAME_FILL) {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFCCCCCC);
        }

        if (render_mode != RENDER_MODE_FILL) {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFFFF);
        }

        if (render_mode == RENDER_MODE_WIREFRAME_POINTS) {
            draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFF0000);
        }
    }

    array_free(triangles_to_render);

    render_colour_buffer();
    clear_colour_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

void free_resources(void) {
    free(colour_buffer);
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
