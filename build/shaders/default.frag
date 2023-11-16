#version 330 core

in vec3 normal;
in vec2 uv;
in float layer;

out vec4 fragColor;

uniform int layers;
uniform float time;

float hash(uint x)
{
    x = (x << 13) ^ x;
    float t = float((x * (x * x * 15731u + 789221u) + 1376312589u) & 0x7fffffffu);    
    return 1.0f - (t / 1073741824.0f);
}

void main()
{    
    const vec3 light_direction = vec3(0.58, 0.58, 0.58);
    const float density = 150.0;
    const float attenuation = 1.2;
    const float ambient_bias = 0.1;
    const float thickness = 2.0;

    // @Note: Layer height
    float h = layer / layers;
    
    // @Note: Half lambert
    float light = clamp(dot(light_direction, normal), 0, 1) * 0.5 + 0.5;
    light = light * light;
        
    // @Note: Shell texturing at its core
    vec2 new_uv = uv * density;
    uint seed = uint(new_uv.x + 100) * uint(new_uv.y + 100) * uint(10);
    float rand = mix(0.9, 1.1, hash(seed));

    // @Note: Not square-like shell texturing
    vec2 local_uv = fract(new_uv) * 2.0 - 1.0;
    float distance_from_center = length(local_uv);
    float circ_radius = (thickness * (rand - h));

    if (distance_from_center > circ_radius && layer > 0.0) {
        discard;
    }

    const vec3 color_top = vec3(0.20, 0.90, 0.0);
    const vec3 color_bot = vec3(0.42, 0.87, 0.0);
    vec3 color = mix(color_bot, color_top, h);

    float ambient = pow(h, attenuation);
    ambient += ambient_bias;
    ambient = clamp(ambient, 0.0, 1.0);
    vec3 ambient_light = mix(vec3(0.41, 0.84, 0.66), vec3(1.0), ambient);

    fragColor = vec4(color * ambient_light * light, 1.0);
}
