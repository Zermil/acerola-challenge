#version 330 core

varying vec3 uv;

uniform samplerCube skybox;

out vec4 fragColor;

void main()
{
  fragColor = texture(skybox, uv);
}
