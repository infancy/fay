#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D offscreen;

void main()
{    
    FragColor = texture(offscreen, vTex);
}