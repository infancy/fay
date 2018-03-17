#version 330 core 
  
layout(location = 0) in vec3 mPos;

out vec2 vTex;

uniform mat4 MVP;

void main()
{
    if(mPos.x < 0.0)
        vTex.x = 0.0;
    else
        vTex.x = 1.0;   
    if(mPos.y < 0.0)
        vTex.y = 0.0;
    else
        vTex.y = 1.0;  

    gl_Position = MVP * vec4(mPos, 1.0);
}