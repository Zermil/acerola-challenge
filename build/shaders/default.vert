#version 330 core

layout (location = 0) in vec3 ver_pos;
layout (location = 1) in vec3 ver_norm;
layout (location = 2) in vec2 ver_uv;
layout (location = 3) in float current_layer;

uniform mat4 projection;
uniform mat4 view;
uniform int layers;
uniform float fur_length;

out vec3 normal;
out vec2 uv;
out float layer;

void main()
{
    const float curvature = 6.9;
    float shell_height = current_layer / layers;
    shell_height = pow(shell_height, fur_length);
    
    vec3 pos = ver_pos;

    // @Note: This is black magic
    // ~Shadow wizard money gang
    pos += ver_norm * fur_length * shell_height;

    float k = pow(shell_height, curvature);
    pos += vec3(0.0, -1.0, 0.0) * 0.1 * k;

    gl_Position = projection * view * vec4(pos, 1.0);
    
    normal = ver_norm;
    uv = ver_uv;
    layer = current_layer;
}
