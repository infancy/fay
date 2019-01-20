#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;

out VS_OUT {
    vec3 vNor;
    vec2 vTex;
} vs_out;

uniform mat3 NormalMV;
uniform mat4 P;
uniform mat4 MVP;

void main()
{
    vs_out.vTex = mTex;    
    vs_out.vNor = normalize(vec3(P * 
        vec4(NormalMV * mNor, 0.0)));
    gl_Position = MVP * vec4(mPos, 1.0);
}