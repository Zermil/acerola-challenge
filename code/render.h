#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

#define FPS 60
#define FRAME_MS (1000/FPS)

#define MAX_QUADS (16 * 1024)
#define MAX_VERTICES (MAX_QUADS * 4)
#define MAX_INDICES (MAX_QUADS * 6)

#define SPHERE_SECTOR_COUNT 64
#define SPHERE_STACK_COUNT 32
#define SPHERE_RADIUS 1

#define PI32 3.1415926535897932f

struct Render_Context {
    SDL_Window *window;
    
    float width;
    float height;
};

// @Note: Dear CS Professors, I don't care - it's way easier with this being global
extern Render_Context global_ctx;

void render_initialize_context();
void render_destroy_context();
void render_begin();
void render_end();
void render_immediate_sphere(float angle);

#endif // RENDER_H
