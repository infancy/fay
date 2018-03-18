#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diff;

void main()
{    
    //FragColor = texture(diff, vTex);
    vec4 c = texture(diff, vTex);
    //FragColor = vec4(c.r, c.r, c.r, 1);
    FragColor = vec4(c.r, c.g, c.b, 1);
}