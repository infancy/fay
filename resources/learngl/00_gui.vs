#version 330 core 
  
layout(location = 0) in vec3 mPos;

out vec2 vTex;

uniform mat4 MVP;

void main()
{
    vTex = mPos.xy;    
    gl_Position = MVP * vec4(mPos, 1.0);
}