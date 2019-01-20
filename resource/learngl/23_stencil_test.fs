#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D Diffuse;
uniform bool draw_outlining;

void main()
{    
    if(!draw_outlining)
        FragColor = texture(Diffuse, vTex);
    else
        FragColor = vec4(1.0);
}