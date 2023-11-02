#include "triangle.h"
#include "display.h"
#include "swap.h"

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t colour) {
    draw_line(x0, y0, x1, y1, colour);
    draw_line(x1, y1, x2, y2, colour);
    draw_line(x2, y2, x0, y0, colour);
}

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t colour) {
    float inv_slope_1 = (float) (x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float) (x2 - x0) / (y2 - y0);

    float x_start = x0;
    float x_end = x0;

    for (int y = y0; y <= y2; ++y) {
        draw_line(x_start, y, x_end, y, colour);

        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t colour) {
    float inv_slope_1 = (float) (x2 - x0) / (y2 - y0);
    float inv_slope_2 = (float) (x2 - x1) / (y2 - y1);

    float x_start = x2;
    float x_end = x2;

    for (int y = y2; y >= y0; --y) {
        draw_line(x_start, y, x_end, y, colour);

        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t colour) {
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 == y2) {
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, colour);
    } else if (y0 == y1) {
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, colour);
    } else {
        // find mid-point using triangle similarity
        int my = y1;
        int mx = (float) ((x2 - x0) * (y1 - y0)) / (float) (y2 - y0) + x0;

        fill_flat_bottom_triangle(x0, y0, x1, y1, mx, my, colour);
        fill_flat_top_triangle(x1, y1, mx, my, x2, y2, colour);
    }
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

void draw_texel(
    int x, int y, uint32_t* texture,
    vec2_t point_a, vec2_t point_b, vec2_t point_c,
    float u0, float v0, float u1, float v1, float u2, float v2
) {
    vec2_t point_p = { .x = x, .y = y };
    vec3_t weights = barycentric_weights(point_a, point_b, point_c, point_p);

    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    float interpolated_u = u0 * alpha + u1 * beta + u2 * gamma;
    float interpolated_v = v0 * alpha + v1 * beta + v2 * gamma;

    int tex_x = abs((int) (texture_width * interpolated_u));
    int tex_y = abs((int) (texture_height * interpolated_v));

    draw_pixel(x, y, texture[tex_y * texture_width + tex_x]);
}

void draw_textured_triangle(
    int x0, int y0, float u0, float v0,
    int x1, int y1, float u1, float v1,
    int x2, int y2, float u2, float v2,
    uint32_t* texture
) {
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    vec2_t point_a = { .x = x0, .y = y0 };
    vec2_t point_b = { .x = x1, .y = y1 };
    vec2_t point_c = { .x = x2, .y = y2 };

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
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
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
                draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
            }
        }
    }
}

int triangle_compare_avg_depth(const void* a, const void* b) {
    triangle_t* triangle_a = (triangle_t*) a;
    triangle_t* triangle_b = (triangle_t*) b;

    return (triangle_a->avg_depth < triangle_b->avg_depth) - (triangle_a->avg_depth > triangle_b->avg_depth);
}
