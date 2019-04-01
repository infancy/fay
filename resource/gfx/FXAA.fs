#version 330 core
in vec2 vTex;
out vec4 FragColor;

layout (std140) uniform color
{
    vec4 a;
    vec4 b;
};

uniform sampler2D frame;

void main()
{    
	FragColor = texture(frame, vTex);
}