#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec2 mTex;

void main()
{
   gl_Position = vec4(mPos.x, mPos.y, mPos.z, 1.0);
}