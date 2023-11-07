#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#include "./linmath.h"
#include "./utils.h"
#include "./render.h"

#define FPS 60
#define FRAME_MS (1000/FPS)

#define ZOOM_RATE 2.8f
#define ZOOM_UP 0.950f
#define ZOOM_DOWN 1.090f
#define ZOOM_MAX 1.5f
#define ZOOM_MIN 3.0f

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    // @Note: Important and error-prone as this has to be called first
    render_initialize_context();
    
    bool should_quit = false;
    float angle = 0.0f;
    
    float zoom_target = global_ctx.camera.eye[2];

    unsigned int current_time = 0;
    unsigned int previous_time = SDL_GetTicks();

    while (!should_quit) {
        current_time = SDL_GetTicks();
        unsigned int elapsed_time = current_time - previous_time;
        previous_time = current_time;

        float dt = elapsed_time / 1000.0f;
        
        SDL_Event e = {0};
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    should_quit = true;
                } break;

                case SDL_MOUSEWHEEL: {
                    if (e.wheel.y > 0.0f) zoom_target *= ZOOM_UP;
                    else if (e.wheel.y < 0.0f) zoom_target *= ZOOM_DOWN;

                    if (zoom_target < ZOOM_MAX) zoom_target = ZOOM_MAX;
                    else if (zoom_target > ZOOM_MIN) zoom_target = ZOOM_MIN;
                } break;

                case SDL_MOUSEMOTION: {
                    unsigned int mouse_state = SDL_GetMouseState(0, 0);
                    if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                        SDL_CaptureMouse(SDL_TRUE);

                        vec2 v = { (float) e.motion.xrel, (float) e.motion.yrel };
                        float mag = vec2_len(v);

                        if (e.motion.xrel > 0) angle += mag * dt;
                        else if (e.motion.xrel < 0) angle -= mag * dt;
                        
                        SDL_CaptureMouse(SDL_FALSE);
                    }
                } break;
            }
        }

        move_towards(&global_ctx.camera.eye[2], zoom_target, dt, ZOOM_RATE);
        
        mat4x4 view = {0};
        mat4x4_identity(view);
        mat4x4_look_at(view, global_ctx.camera.eye, global_ctx.camera.center, global_ctx.camera.up);
        mat4x4_rotate_Y(view, view, angle);
        
        render_update_camera(view);
        
        render_begin();
        render_immediate_sphere(0.0f);
        render_end();

        int wait_time = FRAME_MS - elapsed_time;
        if (wait_time > 0 && wait_time < FRAME_MS) SDL_Delay(wait_time);
    }

    render_destroy_context();
    
    return 0;
}
