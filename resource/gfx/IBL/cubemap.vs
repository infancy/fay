#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

out VS_OUT
{
    vec3 wPos0;
    vec3 wPos1;
    vec3 wPos2;
    vec3 wPos3;
    vec3 wPos4;
    vec3 wPos5;
} vs_out;

uniform mat4 proj;
uniform mat4 view;

uniform mat4 model0;
uniform mat4 model1;
uniform mat4 model2;
uniform mat4 model3;
uniform mat4 model4; // positive z
uniform mat4 model5;

void main()
{
    vs_out.wPos0 = vec3(model0 * vec4(mPos, 1.0));
    vs_out.wPos1 = vec3(model1 * vec4(mPos, 1.0));
    vs_out.wPos2 = vec3(model2 * vec4(mPos, 1.0));
    vs_out.wPos3 = vec3(model3 * vec4(mPos, 1.0));
    vs_out.wPos4 = vec3(model4 * vec4(mPos, 1.0));
    vs_out.wPos5 = vec3(model5 * vec4(mPos, 1.0));
    
    // WARNNING: use wPos5
    gl_Position =  proj * view * vec4(vs_out.wPos4, 1.0);
}