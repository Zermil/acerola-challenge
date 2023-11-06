#version 330 core

layout (location = 0) in vec3 ver_pos;
layout (location = 1) in vec3 ver_norm;
layout (location = 2) in vec2 ver_uv;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 normal;
out vec2 uv;

void main()
{
  gl_Position = projection * view * model * vec4(ver_pos, 1.0);
  
  normal = ver_norm;
  uv = ver_uv;
}