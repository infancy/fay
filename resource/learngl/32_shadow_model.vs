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

uniform mat4 Proj;
uniform mat4 View;
uniform mat4 Model;
uniform mat4 LightSpace;

void main()
{
    gl_Position = Proj * View * Model * vec4(mPos, 1.0f);
    // 在世界空间中做计算
    vs_out.wPos = vec3(Model * vec4(mPos, 1.0));
    vs_out.wNor = transpose(inverse(mat3(Model))) * mNor;
    vs_out.wTex = mTex;
    vs_out.LightSpacePos = LightSpace * vec4(vs_out.wPos, 1.0);
}