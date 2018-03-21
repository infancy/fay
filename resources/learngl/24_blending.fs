#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diffuse;

void main()
{    
    //vec4 texColor = texture(diff, vTex);
    //if(texColor.a < 0.1)
    //    discard;
    //FragColor = texColor;
    FragColor = texture(diffuse, vTex);
}