#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "mesh.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = { .x = 0, .y = 0, .z = 0 }
};

int read_face(FILE* fp, char* buffer) {
    fscanf(fp, "%s", buffer);
    char* slash_char = strchr(buffer, '/');
    if (slash_char != NULL) {
        *slash_char = '\0';
    }
    return atoi(buffer);
}

float read_vertex(FILE* fp) {
    float vertex;
    fscanf(fp, "%f", &vertex);
    return vertex;
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
            vec3_t vertex = {
                .x = read_vertex(fp),
                .y = read_vertex(fp),
                .z = read_vertex(fp)
            };
            array_push(mesh.vertices, vertex);
        } else if (strcmp(buffer, "f") == 0) {
            face_t face = {
                .a = read_face(fp, buffer),
                .b = read_face(fp, buffer),
                .c = read_face(fp, buffer)
            };
            array_push(mesh.faces, face);
        }
    }

    fclose(fp);

    return true;
}
