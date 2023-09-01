#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint32_t* colour_buffer;
extern SDL_Texture* colour_buffer_texture;
extern int window_width;
extern int window_height;

bool initialise_window(void);
void draw_grid(void);
void draw_pixel(int x, int y, uint32_t colour);
void draw_rect(int start_x, int start_y, int width, int height, uint32_t colour);
void render_colour_buffer(void);
void clear_colour_buffer(uint32_t colour);
void destroy_window(void);

#endif
