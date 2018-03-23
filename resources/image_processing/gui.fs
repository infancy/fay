#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diffuse;

void main()
{    
    //FragColor = texture(diffuse, vTex);
    vec4 c = texture(diffuse, vTex);
    //FragColor = vec4(c.r, c.r, c.r, 1);
    FragColor = vec4(c.r, c.g, c.b, 1);
}