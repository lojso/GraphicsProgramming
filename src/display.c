#include "display.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint32_t *color_buffer = NULL;
SDL_Texture *color_buffer_texture = NULL;
int window_width = 800;
int window_height = 600;

bool initialize_window(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    window_width = display_mode.w;
    window_height = display_mode.h;
    // Create a SDL Window
    window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_BORDERLESS);

    if (!window)
    {
        fprintf(stderr, "Error creating SDL window\n");
        return false;
    }
    // Create a SDL Renderer
    renderer = SDL_CreateRenderer(window, -1, 0);
    return true;

    if (!renderer)
    {
        fprintf(stderr, "Error creating SDL renderer\n");
        return false;
    }
}

void draw_grid()
{
    for (int y = 0; y < window_height; y += 10)
    {
        for (int x = 0; x < window_width; x += 10)
        {
            color_buffer[(window_width * y) + x] = 0xFF444444;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color)
{
    if ((x < window_width && y < window_height) && (x >= 0 && y >= 0))
    {
        color_buffer[(window_width * y) + x] = color;
    }
}

void draw_line_vec2(vec2_t p0, vec2_t p1, uint32_t color)
{
    int delta_x = p1.x - p0.x;
    int delta_y = p1.y - p0.y;

    int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

    float x_inc = delta_x / (float)longest_side_length;
    float y_inc = delta_y / (float)longest_side_length;

    float current_x = p0.x;
    float current_y = p0.y;

    for (int i = 0; i <= longest_side_length; i++)
    {
        draw_pixel(round(current_x), round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    vec2_t p0 = {
        .x = x0,
        .y = y0,
    };

    vec2_t p1 = {
        .x = x1,
        .y = y1,
    };

    draw_line_vec2(p0, p1, color);
}

void draw_rectangle(int x, int y, int width, int height, uint32_t color)
{
    for (int i = x; i < x + width; i++)
    {
        for (int j = y; j < y + height; j++)
        {
            int current_x = i;
            int current_y = j;
            draw_pixel(current_x, current_y, color);
        }
    }
}

void render_color_buffer(void)
{
    SDL_UpdateTexture(color_buffer_texture, NULL, color_buffer, (int)(window_width * sizeof(uint32_t)));
    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color)
{
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            color_buffer[window_width * y + x] = color;
        }
    }
}

void destroy_window(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
