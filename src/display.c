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
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            if (x % 100 == 0 || y % 100 == 0)
                color_buffer[window_width * y + x] = 0xFF111111;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color)
{
    if ((x < window_width && y <window_height) && (x >= 0 && y >= 0))
    {
        color_buffer[(window_width * y) + x] = color;
    }
    
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
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
