#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

#define FPS 30
#define FRAME_TARGET_TIME (1000 / FPS)

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE
};

enum render_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
};

bool initialise_window(void);
int get_window_width(void);
int get_window_height(void);
void set_render_method(int render_method);
void set_cull_method(int cull_method);
bool is_cull_backface(void);
bool should_render_filled_triangles(void);
bool should_render_textured_triangles(void);
bool should_render_wireframe(void);
bool should_render_wire_vertex(void);
void draw_grid(void);
void draw_pixel(int x, int y, uint32_t colour);
void draw_line(int x0, int y0, int x1, int y1, uint32_t colour);
void draw_rect(int start_x, int start_y, int width, int height, uint32_t colour);
void render_colour_buffer(void);
void clear_colour_buffer(uint32_t colour);
void clear_z_buffer(void);
float get_z_buffer_at(int x, int y);
void update_z_buffer_at(int x, int y, float value);
void destroy_window(void);

#endif
