#version 330 core

in vec3 normal;
in vec2 uv;

out vec4 fragColor;

void main()
{
  vec3 light_direction = vec3(0.58, 0.58, 0.58);

  // Half lambert
  float light = clamp(dot(light_direction, normal), 0, 1) * 0.5 + 0.5;
  light = light * light;
  
  fragColor = vec4(vec3(light), 1.0);
}