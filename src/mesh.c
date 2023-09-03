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
            face_t face;
            fscanf(fp, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &face.a, &face.b, &face.c);
            array_push(mesh.faces, face);
        }
    }

    fclose(fp);

    return true;
}
