#version 330 core

in vec3 normal;
in vec2 uv;

out vec4 fragColor;

float hash(uint x)
{
    x = (x << 13) ^ x;
    float t = float((x * (x * x * 15731u + 789221u) + 1376312589u) & 0x7fffffffu);    
    return 1.0f - (t / 1073741824.0f);
}

void main()
{
    vec3 light_direction = vec3(0.58, 0.58, 0.58);
    
    // @Note: Half lambert
    float light = clamp(dot(light_direction, normal), 0, 1) * 0.5 + 0.5;
    light = light * light;
    
    fragColor = vec4(vec3(light), 1.0);
}
