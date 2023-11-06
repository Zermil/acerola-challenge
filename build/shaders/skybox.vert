#version 330 core

layout (location = 0) in vec3 ver_pos;

out vec3 uv;

void main()
{
  vec4 pos = vec4(ver_pos, 1.0);
  gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
  
  uv = vec3(ver_pos.x, ver_pos.y, ver_pos.z);
}