#include <stdlib.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#include "./utils.h"
#include "./render.h"

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    render_initialize_context();
    
    bool should_quit = false;
    unsigned int current_time = 0;
    unsigned int previous_time = SDL_GetTicks();

    float angle = 0.0f;
    
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
        
        if (elapsed_time < FRAME_MS) SDL_Delay(FRAME_MS - elapsed_time);
    }

    render_destroy_context();
    
    return 0;
}
