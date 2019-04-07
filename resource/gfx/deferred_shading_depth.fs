#version 330 core
in vec3 vPos;
in vec3 vNor;
in vec2 vTex;

layout (location = 3) out vec4 gDepth;

uniform bool bFront;

void main()
{   
    if(bFront)
    {
        gDepth.r = gl_FragCoord.z;
    }
    else
    {
        gDepth.g = gl_FragCoord.z;
    }
}