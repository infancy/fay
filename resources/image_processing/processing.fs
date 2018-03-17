#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diff;

void main()
{    
    FragColor = texture(diff, vTex);
}