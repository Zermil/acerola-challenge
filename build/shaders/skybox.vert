#version 330 core

attribute vec3 ver_pos;

uniform mat4 projection;
uniform mat4 view;

varying vec3 uv;

void main()
{
  vec4 pos = projection * view * vec4(ver_pos, 1.0);
  gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
  
  uv = vec3(ver_pos.x, ver_pos.y, ver_pos.z);
}