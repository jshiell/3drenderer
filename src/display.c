#include <math.h>
#include "display.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

static uint32_t* colour_buffer = NULL;
static float* z_buffer = NULL;

static SDL_Texture* colour_buffer_texture = NULL;
static int window_width = 800;
static int window_height = 600;

static int render_method = RENDER_WIRE;
static int cull_method = CULL_NONE;

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

    colour_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);
    z_buffer = (float*) malloc(sizeof(float) * window_width * window_height);

    colour_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    return true;
}

int get_window_width(void) {
    return window_width;
}

int get_window_height(void) {
    return window_height;
}

void set_render_method(int method) {
    render_method = method;
}

void set_cull_method(int method) {
    cull_method = method;
}

bool is_cull_backface(void) {
    return cull_method == CULL_BACKFACE;
}

bool should_render_filled_triangles(void) {
    return render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE;
}

bool should_render_textured_triangles(void) {
    return render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE;
}

bool should_render_wireframe(void) {
    return render_method == RENDER_WIRE 
        || render_method == RENDER_WIRE_VERTEX
        || render_method == RENDER_FILL_TRIANGLE_WIRE
        || render_method == RENDER_TEXTURED_WIRE;
}

bool should_render_wire_vertex(void) {
    return render_method == RENDER_WIRE_VERTEX;
}

void draw_grid(void) {
    for (int y = 0; y < window_height; y += 10) {
        for (int x = 0; x < window_width; x += 10) {
            colour_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }
}

void draw_pixel(int x, int y, uint32_t colour) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
        return;
    }
    colour_buffer[(window_width * y) + x] = colour;
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
    SDL_RenderPresent(renderer);
}

void clear_colour_buffer(uint32_t colour) {
    for (int i = 0; i < window_width * window_height; ++i) {
        colour_buffer[i] = colour;
    }
}

void clear_z_buffer(void) {
    for (int i = 0; i < window_width * window_height; ++i) {
        z_buffer[i] = 1.0;
    }
}

float get_z_buffer_at(int x, int y) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
        return 1.0f;
    }
    return z_buffer[(window_width * y) + x];
}

void update_z_buffer_at(int x, int y, float value) {
    if (x < 0 || x >= window_width || y < 0 || y >= window_height) {
        return;
    }
    z_buffer[(window_width * y) + x] = value;
}

void destroy_window(void) {
    free(z_buffer);
    free(colour_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
