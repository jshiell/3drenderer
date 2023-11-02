#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define FPS 30
#define FRAME_TARGET_TIME (1000 / FPS)

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE
} cull_method;

enum render_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
} render_method;

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint32_t* colour_buffer;
extern SDL_Texture* colour_buffer_texture;
extern int window_width;
extern int window_height;

bool initialise_window(void);
void draw_grid(void);
void draw_pixel(int x, int y, uint32_t colour);
void draw_line(int x0, int y0, int x1, int y1, uint32_t colour);
void draw_rect(int start_x, int start_y, int width, int height, uint32_t colour);
void render_colour_buffer(void);
void clear_colour_buffer(uint32_t colour);
void destroy_window(void);

#endif
