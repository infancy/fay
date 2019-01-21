#version 330 core 
layout(location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

out vec2 vTex;

uniform mat4 MVP;

void main()
{
    vTex = mTex.yx; //mPos.xy;    
    gl_Position = MVP * vec4(mPos, 1.0);
}