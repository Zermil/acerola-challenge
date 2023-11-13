#version 330 core

in vec3 normal;
in vec2 uv;
in float layer;

out vec4 fragColor;

uniform int layers;

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
    const float attenuation = 1.3;
    const float ambient_bias = 0.1;
    const float thickness = 1.8;
    
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

    if (distance_from_center > (thickness * (rand - h)) && layer > 0.0) {
        discard;
    }

    // @Note: 'Ambient occlusion' (tm)
    float ambient = pow(h, attenuation) + ambient_bias;
    ambient = clamp(ambient, 0.0, 1.0);

    fragColor = vec4(vec3(0.0, 0.98, 0.0) * ambient * vec3(light), 1.0);
}
