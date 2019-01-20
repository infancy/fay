#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;

out vec2 vTex;

layout (std140) uniform Mat
{
    mat4 proj;
    mat4 view;
};

uniform mat4 model;

void main()
{
    vTex = mTex;    
    gl_Position = proj * view * model * vec4(mPos, 1.0);
}