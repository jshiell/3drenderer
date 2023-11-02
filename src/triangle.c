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
                draw_pixel(x, y, 0xFFFF00FF);
            }
        }
    }
}

int triangle_compare_avg_depth(const void* a, const void* b) {
    triangle_t* triangle_a = (triangle_t*) a;
    triangle_t* triangle_b = (triangle_t*) b;

    return (triangle_a->avg_depth < triangle_b->avg_depth) - (triangle_a->avg_depth > triangle_b->avg_depth);
}
