#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D Diffuse;

void main()
{    
    FragColor = texture(Diffuse, vTex);
}