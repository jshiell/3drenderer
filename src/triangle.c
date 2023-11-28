#include "triangle.h"
#include "display.h"
#include "swap.h"

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t colour) {
    draw_line(x0, y0, x1, y1, colour);
    draw_line(x1, y1, x2, y2, colour);
    draw_line(x2, y2, x0, y0, colour);
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    vec2_t ac = vec2_sub(c, a);
    vec2_t ab = vec2_sub(b, a);
    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);
    vec2_t ap = vec2_sub(p, a);

    float area_parallelogram_abc = (ac.x * ab.y - ac.y * ab.x); // || AC x AB ||
    float alpha = (pc.x * pb.y - pc.y * pb.x) / area_parallelogram_abc; // || PC x PB || / || AC x AB ||
    float beta = (ac.x * ap.y - ac.y * ap.x) / area_parallelogram_abc; // || AC x AP || / || AC x AB ||
    float gamma = 1.0f - alpha - beta;

    vec3_t weights = {
        .x = alpha,
        .y = beta,
        .z = gamma
    };
    return weights;
}

void draw_triangle_pixel(
    int x, int y, int32_t colour,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    float recipricol_w_a, float recipricol_w_b, float recipricol_w_c
) {
    vec2_t p = { .x = x, .y = y };
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    float interpolated_recipricol_w = recipricol_w_a * alpha + recipricol_w_b * beta + recipricol_w_c * gamma;

    float z_buffer_w = 1.0 - interpolated_recipricol_w;
    if (z_buffer_w < get_z_buffer_at(x, y)) {
        draw_pixel(x, y, colour);
        update_z_buffer_at(x, y, z_buffer_w);
    }
}

void draw_filled_triangle(
    int x0, int y0, float z0, float w0,
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2,
    uint32_t colour
) {
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
    }

    vec4_t point_a = { .x = x0, .y = y0, .z = z0, .w = w0 };
    vec4_t point_b = { .x = x1, .y = y1, .z = z1, .w = w1  };
    vec4_t point_c = { .x = x2, .y = y2, .z = z2, .w = w2  };

    float recipricol_w_a = 1.0 / point_a.w;
    float recipricol_w_b = 1.0 / point_b.w;
    float recipricol_w_c = 1.0 / point_c.w;

    // render flat-bottom triangle
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) inv_slope_1 = (float) (x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float) (x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; ++y) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; ++x) {
                draw_triangle_pixel(
                    x, y, colour,
                    point_a, point_b, point_c,
                    recipricol_w_a, recipricol_w_b, recipricol_w_c
                );
            }
        }
    }

    // render flat-top triangle
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0) inv_slope_1 = (float) (x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0) inv_slope_2 = (float) (x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; ++y) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; ++x) {
                draw_triangle_pixel(
                    x, y, colour,
                    point_a, point_b, point_c,
                    recipricol_w_a, recipricol_w_b, recipricol_w_c
                );
            }
        }
    }
}

void draw_triangle_texel(
    int x, int y, uint32_t* texture,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    float recipricol_w_a, float recipricol_w_b, float recipricol_w_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv
) {
    vec2_t p = { .x = x, .y = y };
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    float interpolated_u;
    float interpolated_v;
    float interpolated_recipricol_w;

    // find u/w and v/w for each point using weights and factor of 1/w
    interpolated_u = a_uv.u * recipricol_w_a * alpha
        + b_uv.u * recipricol_w_b * beta
        + c_uv.u * recipricol_w_c * gamma;
    interpolated_v = a_uv.v * recipricol_w_a * alpha
        + b_uv.v * recipricol_w_b * beta
        + c_uv.v * recipricol_w_c * gamma;

    interpolated_recipricol_w = recipricol_w_a * alpha + recipricol_w_b * beta + recipricol_w_c * gamma;

    interpolated_u /= interpolated_recipricol_w;
    interpolated_v /= interpolated_recipricol_w;

    int tex_x = abs((int) (texture_width * interpolated_u)) % texture_width;
    int tex_y = abs((int) (texture_height * interpolated_v)) % texture_height;

    float z_buffer_w = 1.0 - interpolated_recipricol_w;
    if (z_buffer_w < get_z_buffer_at(x, y)) {
        draw_pixel(x, y, texture[tex_y * texture_width + tex_x]);
        update_z_buffer_at(x, y, z_buffer_w);
    }
}

void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    uint32_t* texture
) {
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // flip the v component to account for inverted in OBJ files
    v0 = 1.0 - v0;
    v1 = 1.0 - v1;
    v2 = 1.0 - v2;

    vec4_t point_a = { .x = x0, .y = y0, .z = z0, .w = w0 };
    vec4_t point_b = { .x = x1, .y = y1, .z = z1, .w = w1  };
    vec4_t point_c = { .x = x2, .y = y2, .z = z2, .w = w2  };
    tex2_t a_uv = { .u = u0, .v = v0 };
    tex2_t b_uv = { .u = u1, .v = v1 };
    tex2_t c_uv = { .u = u2, .v = v2 };

    float recipricol_w_a = 1.0 / point_a.w;
    float recipricol_w_b = 1.0 / point_b.w;
    float recipricol_w_c = 1.0 / point_c.w;

    // render flat-bottom triangle
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) inv_slope_1 = (float) (x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) inv_slope_2 = (float) (x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0) {
        for (int y = y0; y <= y1; ++y) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; ++x) {
                draw_triangle_texel(
                    x, y, texture,
                    point_a, point_b, point_c,
                    recipricol_w_a, recipricol_w_b, recipricol_w_c,
                    a_uv, b_uv, c_uv
                );
            }
        }
    }

    // render flat-top triangle
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0) inv_slope_1 = (float) (x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0) inv_slope_2 = (float) (x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0) {
        for (int y = y1; y <= y2; ++y) {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start) {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; ++x) {
                draw_triangle_texel(
                    x, y, texture,
                    point_a, point_b, point_c,
                    recipricol_w_a, recipricol_w_b, recipricol_w_c,
                    a_uv, b_uv, c_uv
                );
            }
        }
    }
}
