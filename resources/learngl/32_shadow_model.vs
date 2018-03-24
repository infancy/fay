#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;

out VS_OUT 
{
    vec3 wPos;
    vec3 wNor;
    vec2 wTex;
    vec4 LightSpacePos;
} vs_out;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpace;

void main()
{
    gl_Position = proj * view * model * vec4(mPos, 1.0f);
    // 在世界空间中做计算
    vs_out.wPos = vec3(model * vec4(mPos, 1.0));
    vs_out.wNor = transpose(inverse(mat3(model))) * mNor;
    vs_out.wTex = mTex;
    vs_out.LightSpacePos = lightSpace * vec4(vs_out.wPos, 1.0);
}