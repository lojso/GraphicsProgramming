#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"

triangle_t *triangles_to_render = NULL;

float fov_factor = 640;
vec3_t camera_position = {0, 0, 0};

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
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);
    color_buffer_texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             window_width, window_height);

    load_cube_mesh_data();
    //load_obj_file_data("./assets/cube.obj");
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
        if (event.key.keysym.sym == SDLK_1)
        {
            render_method = RENDER_WIRE_VERTEX;
        }
        if (event.key.keysym.sym == SDLK_2)
        {
            render_method = RENDER_WIRE;
        }
        if (event.key.keysym.sym == SDLK_3)
        {
            render_method = RENDER_FILL_TRIANGLE;
        }
        if (event.key.keysym.sym == SDLK_4)
        {
            render_method = RENDER_FILL_TRIANGLE_WIRE;
        }
        if (event.key.keysym.sym == SDLK_c)
        {
            cull_method = CULL_BACKFACE;
        }
        if (event.key.keysym.sym == SDLK_d)
        {
            cull_method = CULL_NONE;
        }

    default:
        break;
    }
}

void update(void)
{
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME))
        ;

    previous_frame_time = SDL_GetTicks();

    triangles_to_render = NULL;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    mesh.scale.x += 0.002;
    mesh.scale.y += 0.001;

    mesh.translation.x += 0.01;
    mesh.translation.z = 5.0;

    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh.faces[i];
        vec3_t face_verticies[3];
        face_verticies[0] = mesh.vertices[mesh_face.a - 1];
        face_verticies[1] = mesh.vertices[mesh_face.b - 1];
        face_verticies[2] = mesh.vertices[mesh_face.c - 1];

        vec4_t transformed_vertices[3];

        // Transform vertices
        for (int j = 0; j < 3; j++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_verticies[j]);

            // Scale with matrix;
            transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(translation_matrix, transformed_vertex);

            // Constant rotation
            // transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            // transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            // transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);


            transformed_vertices[j] = transformed_vertex;
        }

        //Backface culling check
        if (cull_method == CULL_BACKFACE)
        {
            vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
            vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
            vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

            vec3_t vector_ab = vec3_sub(vector_b, vector_a);
            vec3_t vector_ac = vec3_sub(vector_c, vector_a);
            vec3_normalize(&vector_ab);
            vec3_normalize(&vector_ac);

            vec3_t normal = vec3_cross(vector_ab, vector_ac);
            vec3_normalize(&normal);

            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            float dot_product = vec3_dot(camera_ray, normal);

            if (dot_product < 0)
            {
                // Not projecting face
                // And not pushing it to render
                // If face looks away from camera
                continue;
            }
        }

        vec2_t projected_points[3];

        // Project vertextes on camera frustrum
        for (int j = 0; j < 3; j++)
        {
            projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));

            projected_points[j].x += window_width / 2;
            projected_points[j].y += window_height / 2;
        }

        float avg_depth = (transformed_vertices[0].z +
                           transformed_vertices[1].z +
                           transformed_vertices[2].z) /
                          3;

        triangle_t projected_triangle = {
            .points = {
                {projected_points[0].x, projected_points[0].y},
                {projected_points[1].x, projected_points[1].y},
                {projected_points[2].x, projected_points[2].y},
            },
            .color = mesh_face.color,
            .avg_depth = avg_depth};

        array_push(triangles_to_render, projected_triangle);
    }

    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++)
    {
        for (int j = 0; j < num_triangles; j++)
        {
            if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
            {
                triangle_t temp_triangle = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = temp_triangle;
            }
        }
    }
}

void render(void)
{
    draw_grid();

    int number_of_triangles = array_length(triangles_to_render);
    for (int i = 0; i < number_of_triangles; i++)
    {
        triangle_t triangle = triangles_to_render[i];

        if (render_method == RENDER_FILL_TRIANGLE ||
            render_method == RENDER_FILL_TRIANGLE_WIRE)
        {
            draw_filled_triangle(triangle.points[0].x, triangle.points[0].y,
                                 triangle.points[1].x, triangle.points[1].y,
                                 triangle.points[2].x, triangle.points[2].y,
                                 triangle.color);
        }

        if (render_method == RENDER_WIRE ||
            render_method == RENDER_WIRE_VERTEX ||
            render_method == RENDER_FILL_TRIANGLE_WIRE)
        {
            draw_triangle(triangle.points[0].x, triangle.points[0].y,
                          triangle.points[1].x, triangle.points[1].y,
                          triangle.points[2].x, triangle.points[2].y,
                          0xFFFFFFFF);
        }

        if (render_method == RENDER_WIRE_VERTEX)
        {
            draw_rectangle(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rectangle(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rectangle(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }
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