#version 330 core
in vec2 vTex;
out vec4 FragColor;

layout (std140) uniform color
{
    vec4 a;
    vec4 b;
};

uniform sampler2D Albedo;
uniform bool bAlbedo;

float near = 0.1; 
float far  = 100.0;

void main()
{    
	if(bAlbedo)
    {
    	FragColor = texture(Albedo, vTex);
    }
    else
    	FragColor = vec4(1.f, 0.f, 0.f, 1.f);
}