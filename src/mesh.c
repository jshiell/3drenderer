#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "mesh.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2)

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { .x = 0, .y = 0, .z = 0 },
    .scale = { .x = 1.0, .y = 1.0, .z = 1.0 },
    .translation = { .x = 0, .y = 0, .z = 0 }
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 },
    { .x = -1, .y =  1, .z = -1 },
    { .x =  1, .y =  1, .z = -1 },
    { .x =  1, .y = -1, .z = -1 },
    { .x =  1, .y =  1, .z =  1 },
    { .x =  1, .y = -1, .z =  1 },
    { .x = -1, .y =  1, .z =  1 },
    { .x = -1, .y = -1, .z =  1 }
};

face_t cube_faces [N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3, .colour = 0xFFFF0000 },
    { .a = 1, .b = 3, .c = 4, .colour = 0xFFFF0000 },
    // right
    { .a = 4, .b = 3, .c = 5, .colour = 0xFF00FF00 },
    { .a = 4, .b = 5, .c = 6, .colour = 0xFF00FF00 },
    // back
    { .a = 6, .b = 5, .c = 7, .colour = 0xFF0000FF },
    { .a = 6, .b = 7, .c = 8, .colour = 0xFF0000FF },
    // left
    { .a = 8, .b = 7, .c = 2, .colour = 0xFFFFFF00 },
    { .a = 8, .b = 2, .c = 1, .colour = 0xFFFFFF00 },
    // top
    { .a = 2, .b = 7, .c = 5, .colour = 0xFFFF00FF },
    { .a = 2, .b = 5, .c = 3, .colour = 0xFFFF00FF },
    // bottom
    { .a = 6, .b = 8, .c = 1, .colour = 0xFF00FFFF },
    { .a = 6, .b = 1, .c = 4, .colour = 0xFF00FFFF }
};

void load_cube_mesh_data(void) {
    for (int i = 0; i < N_CUBE_VERTICES; ++i) {
        vec3_t cube_vertix = cube_vertices[i];
        array_push(mesh.vertices, cube_vertix);
    }

    for (int i = 0; i < N_CUBE_FACES; ++i) {
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}

bool load_obj_file_data(char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return false;
    }

    ssize_t read;
    char buffer[32];

    while ((read = fscanf(fp, "%s", buffer)) == 1) {
        if (strcmp(buffer, "v") == 0) {
            vec3_t vertex;
            fscanf(fp, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);
            array_push(mesh.vertices, vertex);
        } else if (strcmp(buffer, "f") == 0) {
            face_t face = {
                .colour = 0xFF555555
            };
            fscanf(fp, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &face.a, &face.b, &face.c);
            array_push(mesh.faces, face);
        }
    }

    fclose(fp);

    return true;
}
