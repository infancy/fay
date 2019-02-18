#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D Diffuse;

layout (std140) uniform color
                {
                    vec4 a;
                    vec4 b;
                };
                
                uniform int flag;

void main()
{    
    FragColor = texture(Diffuse, vTex);
}