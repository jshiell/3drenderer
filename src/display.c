#include <math.h>
#include "display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* colour_buffer = NULL;
SDL_Texture* colour_buffer_texture = NULL;
int window_width = 800;
int window_height = 600;

bool initialise_window(void) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initialising SDL.\n");
        return false;
    }

    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;

    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width,
        window_height,
        SDL_WINDOW_BORDERLESS
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

void draw_grid(void) {
    for (int y = 0; y < window_height; y += 10) {
        for (int x = 0; x < window_width; x += 10) {
            colour_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }
}

void draw_pixel(int x, int y, uint32_t colour) {
    if (x>= 0 && x < window_width && y>=0 && y < window_height) {
        colour_buffer[(window_width * y) + x] = colour;
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t colour) {
    // https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    int delta_x = x1 - x0;
    int delta_y = y1 - y0;

    int longest_side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float) longest_side_length;
    float y_inc = delta_y / (float) longest_side_length;

    float current_x = x0;
    float current_y = y0;
    for (int i = 0; i <= longest_side_length; ++i) {
        draw_pixel(round(current_x), round(current_y), colour);

        current_x += x_inc;
        current_y += y_inc;
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t colour) {
    draw_line(x0, y0, x1, y1, colour);
    draw_line(x1, y1, x2, y2, colour);
    draw_line(x2, y2, x0, y0, colour);
}

void draw_rect(int start_x, int start_y, int width, int height, uint32_t colour) {
    for (int y = start_y; y < start_y + height; ++y) {
        for (int x = start_x; x < start_x + width; ++x) {
            draw_pixel(x, y, colour);
        }
    }
}

void render_colour_buffer(void) {
    SDL_UpdateTexture(
        colour_buffer_texture,
        NULL,
        colour_buffer,
        (int) (window_width * sizeof(uint32_t))
    );

    SDL_RenderCopy(renderer, colour_buffer_texture, NULL, NULL);
}

void clear_colour_buffer(uint32_t colour) {
    for (int y = 0; y < window_height; ++y) {
        for (int x = 0; x < window_width; ++x) {
            colour_buffer[(window_width * y) + x] = colour;
        }
    }
}

void destroy_window(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
