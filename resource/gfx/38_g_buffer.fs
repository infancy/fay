#version 330 core
in vec3 vPos;
in vec3 vNor;
in vec2 vTex;

layout (location = 0) out vec3 gPos;
layout (location = 1) out vec3 gNor;
layout (location = 2) out vec4 gAlbedoSpec;

uniform sampler2D Albedo;
//uniform sampler2D Specular;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPos = vPos;
    // also store the per-fragment normals into the gbuffer
    gNor = normalize(vNor);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(Albedo, vTex).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(Albedo, vTex).r;
}