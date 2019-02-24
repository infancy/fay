#version 330 core

in VS_OUT 
{
    vec3 wPos0;
    vec3 wPos1;
    vec3 wPos2;
    vec3 wPos3;
    vec3 wPos4;
    vec3 wPos5;
} fs_in;

layout (location = 0) out vec3 FragColor0;
layout (location = 1) out vec3 FragColor1;
layout (location = 2) out vec3 FragColor2;
layout (location = 3) out vec3 FragColor3;
layout (location = 4) out vec3 FragColor4;
layout (location = 5) out vec3 FragColor5;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec3 SampleSphericalMap(vec3 v)
{
    v = normalize(v);

    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;

    return texture(equirectangularMap, uv).rgb;
}

void main()
{
    FragColor0 = SampleSphericalMap(fs_in.wPos0);
    FragColor1 = SampleSphericalMap(fs_in.wPos1);
    FragColor2 = SampleSphericalMap(fs_in.wPos2);
    FragColor3 = SampleSphericalMap(fs_in.wPos3);
    FragColor4 = SampleSphericalMap(fs_in.wPos4);
    FragColor5 = SampleSphericalMap(fs_in.wPos5);
}
