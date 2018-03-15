#version 330 core
layout (location = 0) in vec3 mPos;

out vec2 vTex;

// uniform float texture_scale;
uniform bool texture_xz;
uniform mat4 MVP;

void main()
{
    if(texture_xz) vTex = mPos.xz; 
    else vTex = mPos.xy;
    vTex *= 0.98;   // 避免纹理边缘读到另一侧的值
    //vTex *= texture_scale;  
    gl_Position = MVP * vec4(mPos, 1.0);
}