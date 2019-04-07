#version 330 core
in vec3 vPos;
in vec3 vNor;
in vec2 vTex;

layout (location = 0) out vec4 gPos;

uniform bool bFront;

void main()
{   
    gPos.rgb = vPos;
    gPos.a = gl_FragCoord.z;
}