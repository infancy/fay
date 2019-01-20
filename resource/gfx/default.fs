#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D Diffuse;
uniform bool bAlbedo;

void main()
{    
	if(bAlbedo)
    	FragColor = texture(Diffuse, vTex);
    else
    	FragColor = vec4(1.f, 0.f, 0.f, 1.f);
}