#version 330 core
in vec3 vTex;
out vec4 FragColor;

uniform samplerCube Cubemap;

void main()
{    
    FragColor = texture(Cubemap, vTex);
}