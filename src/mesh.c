#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "mesh.h"
#include "upng.h"

#define MAX_NUM_MESHES 10
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

bool load_mesh_obj_data(mesh_t* mesh, char* obj_filename) {
    FILE* fp = fopen(obj_filename, "r");
    if (fp == NULL) {
        return false;
    }

    ssize_t read;
    char buffer[32];
    tex2_t* texcoords = NULL;

    while ((read = fscanf(fp, "%s", buffer)) == 1) {
        if (strcmp(buffer, "v") == 0) {
            vec3_t vertex;
            fscanf(fp, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);
            array_push(mesh->vertices, vertex);

        } else if (strcmp(buffer, "vt") == 0) {
            tex2_t texcoord;
            fscanf(fp, "%f %f", &texcoord.u, &texcoord.v);
            array_push(texcoords, texcoord);

        } else if (strcmp(buffer, "f") == 0) {
            int v_a_index, tc_a_index, v_b_index, tc_b_index, v_c_index, tc_c_index;
            fscanf(fp, "%d/%d/%*d %d/%d/%*d %d/%d/%*d",
                &v_a_index, &tc_a_index,
                &v_b_index, &tc_b_index,
                &v_c_index, &tc_c_index);
            face_t face = {
                .a = v_a_index - 1,
                .b = v_b_index - 1,
                .c = v_c_index - 1,
                .a_uv = texcoords[tc_a_index - 1],
                .b_uv = texcoords[tc_b_index - 1],
                .c_uv = texcoords[tc_c_index - 1],
                .colour = 0xFFFFFFFF
            };
            array_push(mesh->faces, face);
        }
    }

    array_free(texcoords);
    fclose(fp);

    return true;
}

bool load_mesh_png_data(mesh_t* mesh, char* png_filename) {
    upng_t* png_image = upng_new_from_file(png_filename);
    if (png_image != NULL) {
        upng_decode(png_image);
        if (upng_get_error(png_image) == UPNG_EOK) {
            mesh->texture = png_image;
            return true;
        }
    }
    return false;
}

void load_mesh(char* obj_filename, char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation) {
    if (mesh_count == MAX_NUM_MESHES) {
        printf("Maximum number of meshes (%d) reached!\n", MAX_NUM_MESHES);
        return;
    }

    mesh_t* mesh = &meshes[mesh_count];
    mesh->scale = scale;
    mesh->translation = translation;
    mesh->rotation = rotation;

    if (!load_mesh_obj_data(mesh, obj_filename)) {
        printf("Failed to load mesh file: %s\n", obj_filename);
        return;
    }

    if (!load_mesh_png_data(mesh, png_filename)) {
        printf("Failed to load texture file: %s\n", png_filename);
        return;
    }

    ++mesh_count;
}

int get_num_meshes(void) {
    return mesh_count;
}

mesh_t* get_mesh(int index) {
    return &meshes[index];
}

void free_meshes(void) {
    for (int i = 0; i < mesh_count; ++i) {
        upng_free(meshes[i].texture);
        array_free(meshes[i].faces);
        array_free(meshes[i].vertices);
    }
}
