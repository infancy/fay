#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diffuse;
uniform bool inverse;

void main()
{    
    vec4 c = texture(diffuse, vTex);

    if(inverse)
        FragColor = vec4(1 - c.x, 1 - c.y, 1 - c.z, 1.0);
    else
        FragColor = c;
}
