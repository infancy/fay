#version 330 core
layout (location = 0) in vec3 aPos;

layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

uniform mat4 proj;
uniform mat4 view;

out vec3 wPos;

void main()
{
    WorldPos = aPos;  
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}