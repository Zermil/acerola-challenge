#ifndef RENDER_H
#define RENDER_H

#include "./linmath.h"

#include <SDL2/SDL.h>

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

#define MAX_QUADS (32 * 1024)
#define MAX_VERTICES (MAX_QUADS * 4)
#define MAX_INDICES (MAX_QUADS * 6)

#define SPHERE_SECTOR_COUNT 64
#define SPHERE_STACK_COUNT 32

#define LAYERS_COUNT 32
#define FUR_LENGTH 0.01f

struct Render_Context {
    SDL_Window *window;
    
    float width;
    float height;

    bool reload_fail;
};

// @Note: Dear CS Professors, I don't care - it's way easier with this being global
extern Render_Context global_ctx;

void render_initialize_context();
void render_destroy_context();
void render_begin();
void render_end();
void render_reload_shaders();
void render_immediate_sphere();
void render_error_screen();
void render_update_camera(mat4x4 view);

#endif // RENDER_H
