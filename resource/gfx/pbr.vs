#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

out vec2 vTex;
out vec3 wPos;
out vec3 Normal;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vTex = mTex; // or rename to Tex
    wPos = vec3(model * vec4(mPos, 1.0));
    // mat3 normalMatrix = transpose(inverse(mat3(Model)));
    Normal = mat3(model) * mNor;

    gl_Position =  proj * view * vec4(wPos, 1.0);
}