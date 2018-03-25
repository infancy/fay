#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

out VS_OUT 
{
    vec3 wPos;
    vec3 wNor;
    vec2 wTex;
    // 以下三个量都在切线空间
    vec3 tLightPos;
    vec3 tViewPos;
    vec3 tPos;
} vs_out;

uniform mat4 Model;
uniform mat4 MVP;

uniform vec3 LightPos;
uniform vec3 ViewPos;

void main()
{
    vs_out.wPos = vec3(Model * vec4(mPos, 1.0));   
    vs_out.wTex = mTex;
    
    mat3 normalMatrix = transpose(inverse(mat3(Model)));
    vs_out.wNor = normalMatrix * mNor;
    
    vec3 T = normalize(normalMatrix * mTan);
    vec3 N = normalize(normalMatrix * mNor);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    vs_out.tLightPos = TBN * LightPos;
    vs_out.tViewPos  = TBN * ViewPos;
    vs_out.tPos  = TBN * vs_out.wPos;
        
    gl_Position = MVP * vec4(mPos, 1.0);
}