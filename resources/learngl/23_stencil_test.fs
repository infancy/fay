#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diffuse;
uniform bool draw_outlining;

void main()
{    
    if(!draw_outlining)
        FragColor = texture(diffuse, vTex);
    else
        FragColor = vec4(1.0);
}