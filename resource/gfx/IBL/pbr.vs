#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec2 vTex;
out vec3 wPos;
out vec3 wNor;

void main()
{
    vTex = mTex; // or rename to Tex
    wPos = vec3(model * vec4(mPos, 1.0));
    wNor = transpose(inverse(mat3(model))) * mNor;
    //wNor = mat3(model) * mNor; // if not scale or rotate, it's ok

    gl_Position =  proj * view * vec4(wPos, 1.0);
}