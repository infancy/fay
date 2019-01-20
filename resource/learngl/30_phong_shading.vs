#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;

out vec3 vPos;
out vec3 vNor;
out vec2 vTex;

uniform mat4 MV;
uniform mat3 NormalMV;
uniform mat4 MVP;

void main()
{
    vPos = vec3(MV * vec4(mPos, 1.0));
    vNor = NormalMV * mNor;
    vTex = mTex;
    gl_Position = MVP * vec4(mPos, 1.0);
}