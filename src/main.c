#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

triangle_t *triangles_to_render = NULL;

float fov_factor = 640;
vec3_t camera_position = {0, 0, -5};

bool is_running = false;
int previous_frame_time = 0;

vec2_t project(vec3_t point)
{
    vec2_t projected_point = {
        .x = fov_factor * point.x / point.z,
        .y = fov_factor * point.y / point.z};
    return projected_point;
}

void setup(void)
{
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    //load_cube_mesh_data();
    load_obj_file_data("./assets/f22.obj");
}

void process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
        is_running = false;
        break;

    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
            is_running = false;
        }

    default:
        break;
    }
}

void update(void)
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));

    previous_frame_time = SDL_GetTicks();

    triangles_to_render = NULL;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.00;
    mesh.rotation.z += 0.00;

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh.faces[i];
        vec3_t face_verticies[3];
        face_verticies[0] = mesh.vertices[mesh_face.a - 1];
        face_verticies[1] = mesh.vertices[mesh_face.b - 1];
        face_verticies[2] = mesh.vertices[mesh_face.c - 1];

        triangle_t projected_triangle;

        for (int j = 0; j < 3; j++)
        {
            vec3_t transformed_vertex = face_verticies[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            //Translate vertex from camera;
            transformed_vertex.z -= camera_position.z;

            vec2_t projected_point = project(transformed_vertex);

            projected_point.x += window_width / 2;
            projected_point.y += window_height / 2;

            projected_triangle.points[j] = projected_point;
        }
        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void)
{
    int number_of_triangles = array_length(triangles_to_render);
    for (int i = 0; i < number_of_triangles; i++)
    {
        triangle_t triangle = triangles_to_render[i];
        draw_rectangle(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
        draw_rectangle(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
        draw_rectangle(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);

        draw_triangle(triangle.points[0].x, triangle.points[0].y,
                      triangle.points[1].x, triangle.points[1].y,
                      triangle.points[2].x, triangle.points[2].y,
                      0xFF00FF00);
    }

    array_free(triangles_to_render);

    render_color_buffer();

    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

void free_resources()
{
    free(color_buffer);
    array_free(mesh.vertices);
    array_free(mesh.faces);
}

int main(int argc, char *args[])
{
    is_running = initialize_window();

    setup();

    while (is_running)
    {
        process_input();
        update();
        render();
    }



    destroy_window();
    free_resources();

    return 0;
}