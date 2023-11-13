#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <glad/glad.h>

#include "./utils.h"
#include "./render.h"
#include "./stb_image.h"

#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_sdl2.h"
#include "./imgui/imgui_impl_opengl3.h"

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;

    float current_layer;
};

struct Render_Internal {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    unsigned int vao_skybox;
    unsigned int vbo_skybox;
    unsigned int ebo_skybox;
    
    unsigned int shader_default;
    unsigned int shader_skybox;
    
    unsigned int texture_skybox;

    Vertex vertices[MAX_VERTICES];
    size_t vertices_size;

    unsigned int indices[MAX_INDICES];
    size_t indices_size;

    // @Note: This could be in some UI state or something similar
    // instead of being here in render state -- render state is here
    // because it's easier to work with opengl while also having some internal state as well
    float fur_length;
    int layers;
};

Render_Context global_ctx = {0};

// @Note: Dear CS Professors, I don't care - it's way easier with this being global
global Render_Internal state = {0};

internal SDL_Window *render_create_window(const char *window_name, unsigned int w, unsigned int h)
{    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "[ERROR] :: Could not initialize SDL2: %s\n", SDL_GetError());
        exit(1);
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    SDL_Window *window = SDL_CreateWindow(window_name,
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          w, h,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == 0) {
        fprintf(stderr, "[ERROR] :: Not enough memory to initialize SDL window: %s\n", SDL_GetError());
        exit(1);
    }

    global_ctx.gl_context = SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        fprintf(stderr, "[ERROR] :: Failed to initialize OpenGL context: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GL_MakeCurrent(window, global_ctx.gl_context);
    SDL_GL_SetSwapInterval(0);

    // @Note: Enable ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, global_ctx.gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    printf("Window initialized\n");
    printf("------------------\n");
    printf("Version: %s\n", glGetString(GL_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    
    return(window);
}

internal void render_push_index(unsigned int i)
{
    assert(state.indices_size < MAX_INDICES);
    state.indices[state.indices_size] = i;
    state.indices_size += 1;
}

internal void render_push_vertex(Vertex v)
{
    assert(state.vertices_size < MAX_VERTICES);
    state.vertices[state.vertices_size] = v;
    state.vertices_size += 1;
}

internal void render_generate_skybox()
{
    glActiveTexture(GL_TEXTURE0);
    
    glGenTextures(1, &state.texture_skybox);
    glBindTexture(GL_TEXTURE_CUBE_MAP, state.texture_skybox);
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    const char *faces[] = {
        "./assets/skybox/right.png",
        "./assets/skybox/left.png",
        "./assets/skybox/top.png",
        "./assets/skybox/bottom.png",
        "./assets/skybox/front.png",
        "./assets/skybox/back.png"
    };
    
    for (unsigned int i = 0; i < 6; ++i) {
        int w, h, channels;
        unsigned char *data = stbi_load(faces[i], &w, &h, &channels, 0);

        if (data == 0) {
            fprintf(stderr, "[ERROR] :: Could not load texture: %s\n", faces[i]);
            exit(1);
        }
    
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data); // @Robustness: This is kinda silly
    }    
}

// @Note: https://www.songho.ca/opengl/gl_sphere.html
// we're mapping (r, theta, phi) -> (x, y, z) which is just using
// spherical coordinates in a smart way.
//
// Also more info: https://www.youtube.com/watch?v=RkuBWEkBrZA
internal void render_generate_sphere_data(float radius, unsigned int sectors, unsigned int stacks, float current_layer)
{
    const float sector_step = 2.0f * PI32 / sectors;
    const float stack_step = PI32 / stacks;
    const float ilen = 1.0f / radius;

    // @Note: Generate vertices
    for (unsigned int i = 0; i <= stacks; ++i) {
        const float stack_angle = PI32 / 2.0f - i * stack_step;
        const float rcos = radius * cosf(stack_angle);

        float z = radius * sinf(stack_angle);
        
        for (unsigned int j = 0; j <= sectors; ++j) {
            const float sector_angle = j * sector_step;

            float x = rcos * cosf(sector_angle);
            float y = rcos * sinf(sector_angle);

            Vertex v = {
                { x, y, z }, // position
                { x * ilen, y * ilen, z * ilen }, // normal
                { (float) j / sectors, (float) i / stacks }, // uv
                current_layer // current_layer
            };
            
            render_push_vertex(v);
        }
    }
}

internal void render_generate_sphere_indices(unsigned int sectors, unsigned int stacks)
{
    // @Note: Generate indices
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + (sectors + 1);
        
        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                render_push_index(k1);
                render_push_index(k2);
                render_push_index(k1 + 1);
            }

            if (i != (stacks - 1)) {
                render_push_index(k1 + 1);
                render_push_index(k2);
                render_push_index(k2 + 1);
            }
        }
    }
}

internal void render_init_sphere(unsigned int *vao, unsigned int *vbo, unsigned int *ebo)
{
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), state.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(unsigned int), state.indices, GL_STATIC_DRAW);

    // POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // NORMAL
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);

    // CURRENT_LAYER
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, current_layer));
    glEnableVertexAttribArray(3);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

internal void render_init_skybox(unsigned int *vao, unsigned int *vbo, unsigned int *ebo)
{
    float vertices[] = {
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f
    };

    unsigned int indices[] = {
        1, 2, 6,
        6, 5, 1,
        0, 4, 7,
        7, 3, 0,
        4, 5, 6,
        6, 7, 4,
        
        0, 3, 2,
        2, 1, 0,
        0, 1, 5,
        5, 4, 0,
        3, 7, 6,
        6, 2, 3
    };
    
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // POSITION
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

internal bool render_init_shader(const char *vert, const char *frag, unsigned int *shader)
{
    const char *vert_src = load_entire_file(vert);
    const char *frag_src = load_entire_file(frag);
    
    char error_log[512] = {0};
    int success;
    
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vert_src, 0);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, 0, error_log);
        fprintf(stderr, "[SHADER ERROR] :: Could not compile vertex shader \'%s\':\n\t%s", vert, error_log);
        return(false);
    }
    
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &frag_src, 0);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, 0, error_log);
        fprintf(stderr, "[SHADER ERROR] :: Could not compile fragment shader \'%s\':\n\t%s", frag, error_log);
        return(false);
    }

    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex);
    glAttachShader(shader_program, fragment);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, 0, error_log);
        fprintf(stderr, "[SHADER ERROR] :: Could not link shaders \'%s\' & \'%s\':\n\t%s", vert, frag, error_log);
        return(false);
    }
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    *shader = shader_program;
    
    return(true);
}

internal void render_init_projection(float w, float h)
{
    mat4x4 projection = {0};
    mat4x4_identity(projection);
    mat4x4_perspective(projection, 1.74f, w/h, 0.1f, 100.0f);
    
    glUseProgram(state.shader_default);
    glUniformMatrix4fv(glGetUniformLocation(state.shader_default, "projection"), 1, GL_FALSE, &projection[0][0]);

    glUseProgram(state.shader_skybox);
    glUniformMatrix4fv(glGetUniformLocation(state.shader_skybox, "projection"), 1, GL_FALSE, &projection[0][0]);

    glUseProgram(0);
}

internal void render_prepare_default_uniform_values()
{
    state.fur_length = FUR_LENGTH;
    state.layers = LAYERS_COUNT;

    glUseProgram(state.shader_default);
    glUniform1i(glGetUniformLocation(state.shader_default, "layers"), state.layers); 
    glUniform1f(glGetUniformLocation(state.shader_default, "fur_length"), state.fur_length);
    glUseProgram(0);
}

void render_initialize_context()
{
    global_ctx.width = WIN_WIDTH;
    global_ctx.height = WIN_HEIGHT;
    global_ctx.window = render_create_window("A window", WIN_WIDTH, WIN_HEIGHT);
    
    // @Note: Indices are always the same as they depend only on stacks and segments
    // meaning we can just precompute them and dynamically append sphere vertices later.
    render_generate_sphere_indices(SPHERE_SECTOR_COUNT, SPHERE_STACK_COUNT);
    render_generate_skybox();
    
    render_init_sphere(&state.vao, &state.vbo, &state.ebo);
    render_init_skybox(&state.vao_skybox, &state.vbo_skybox, &state.ebo_skybox);

    render_reload_shaders();
    render_init_projection(global_ctx.width, global_ctx.height);
    render_prepare_default_uniform_values();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render_destroy_context()
{
    SDL_GL_DeleteContext(global_ctx.gl_context);
    SDL_DestroyWindow(global_ctx.window);

    glDeleteVertexArrays(1, &state.vao);
    glDeleteBuffers(1, &state.vbo);
    glDeleteBuffers(1, &state.ebo);
    
    glDeleteVertexArrays(1, &state.vao_skybox);
    glDeleteBuffers(1, &state.vbo_skybox);
    glDeleteBuffers(1, &state.ebo_skybox);
    
    glDeleteTextures(1, &state.texture_skybox);
    
    glDeleteProgram(state.shader_default);
    glDeleteProgram(state.shader_skybox);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_Quit();
}

void render_begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    
    glClearColor(0.07f, 0.07f, 0.07f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_end()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    SDL_GL_SwapWindow(global_ctx.window);
}

void render_reload_shaders()
{
    // @Note: This could be a loop if this grows larger
    bool load_default = render_init_shader("./shaders/default.vert", "./shaders/default.frag", &state.shader_default);
    bool load_skybox = render_init_shader("./shaders/skybox.vert", "./shaders/skybox.frag", &state.shader_skybox);

    if (load_default && load_skybox) {
        global_ctx.reload_fail = false;
    } else {
        global_ctx.reload_fail = true;
    }
}

void render_sphere_controls()
{
    ImGui::Begin("Controls");
    {
        glUseProgram(state.shader_default);
        
        if (ImGui::SliderFloat("Fur length", &state.fur_length, 0.25f, 0.5f)) {
            glUniform1f(glGetUniformLocation(state.shader_default, "fur_length"), state.fur_length);
        }

        if (ImGui::SliderInt("Layers", &state.layers, 8, LAYERS_COUNT)) {
            glUniform1i(glGetUniformLocation(state.shader_default, "layers"), state.layers); 
        }
        
        glUseProgram(0);
    }
    ImGui::End();
}

void render_immediate_sphere()
{    
    // @Note: Sphere rendering
    glUseProgram(state.shader_default);

    glBindVertexArray(state.vao);

    for (unsigned int i = 0; i < LAYERS_COUNT; ++i) {
        state.vertices_size = 0;
        render_generate_sphere_data(SPHERE_RADIUS, SPHERE_SECTOR_COUNT, SPHERE_STACK_COUNT, (float) i);
        
        glBindBuffer(GL_ARRAY_BUFFER, state.vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, state.vertices_size * sizeof(Vertex), state.vertices);
        
        glDrawElements(GL_TRIANGLES, (GLsizei) state.indices_size, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    
    // @Note: Skybox rendering
    glUseProgram(state.shader_skybox);
    
    // @Note: Skybox should be rendered after everything for better performance
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(state.vao_skybox);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, state.texture_skybox);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    
    glDepthFunc(GL_LESS);
    
    glBindVertexArray(0);
}

void render_error_screen()
{
    glClearColor(1.0f, 0.16f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_update_camera(mat4x4 view)
{
    glUseProgram(state.shader_default);
    glUniformMatrix4fv(glGetUniformLocation(state.shader_default, "view"), 1, GL_FALSE, &view[0][0]);

    // @Note: Remove camera translation for skybox
    view[3][0] = view[0][3] = 0.0f;
    view[3][1] = view[1][3] = 0.0f;
    view[3][2] = view[2][3] = 0.0f;
    view[3][3] = 0.0f;

    glUseProgram(state.shader_skybox);
    glUniformMatrix4fv(glGetUniformLocation(state.shader_skybox, "view"), 1, GL_FALSE, &view[0][0]);
}

void render_resize_window(int width, int height)
{
    global_ctx.width = (float) width;
    global_ctx.height = (float) height;
    
    glViewport(0, 0, width, height);
    render_init_projection(global_ctx.width, global_ctx.height);
}
