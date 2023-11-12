#version 330 core

in vec3 normal;
in vec2 uv;
in float layer;

out vec4 fragColor;

uniform float layers;

float hash(uint x)
{
    x = (x << 13) ^ x;
    float t = float((x * (x * x * 15731u + 789221u) + 1376312589u) & 0x7fffffffu);    
    return 1.0f - (t / 1073741824.0f);
}

void main()
{
    const vec3 light_direction = vec3(0.58, 0.58, 0.58);
    const float density = 100.0;

    // @Note: Half lambert
    float light = clamp(dot(light_direction, normal), 0, 1) * 0.5 + 0.5;
    light = light * light;

    // @Note: Shell texturing at its core
    vec2 new_uv = uv * density;
    uint seed = uint(new_uv.x + 100) * uint(new_uv.y + 100) * uint(10);
    float rand = mix(10.0, 150.0, hash(seed));
    float h = layer / layers;
    
    vec3 colour = vec3(0.0);
    if (rand > h * h) colour.g = 1.0;
    else discard;
    
    fragColor = vec4(colour * vec3(light), 1.0);
}
