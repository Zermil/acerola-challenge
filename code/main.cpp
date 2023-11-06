#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#include "./utils.h"
#include "./render.h"

#define FPS 60
#define FRAME_MS (1000/FPS)

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    render_initialize_context();
    
    bool should_quit = false;
    float angle = 0.0f;

    float elapsed_secs = 0.0f;
    unsigned int frame_count = 0;
    
    unsigned int current_time = 0;
    unsigned int previous_time = SDL_GetTicks();
    unsigned int update_time = SDL_GetTicks() + FRAME_MS;

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
            }
        }
        
        render_begin();
        render_immediate_sphere(angle);
        render_end();

        angle += 0.7f * dt;

        elapsed_secs += dt;
        if (elapsed_secs > 0.25f) {
            float fps = frame_count / elapsed_secs;
            printf("\rFPS: %.2f", fps);
            
            elapsed_secs = 0.0f;
            frame_count = 0;
        }
        frame_count += 1;

        if (current_time < update_time) SDL_Delay(update_time - current_time);
        update_time += FRAME_MS;
    }

    render_destroy_context();
    
    return 0;
}
