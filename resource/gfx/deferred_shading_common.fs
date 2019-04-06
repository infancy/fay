#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D gPosition;

uniform bool bAlbedo;
uniform sampler2D Diffuse;
uniform vec3 diffuse;

void main()
{    
	//if(gl_FragCoord.z <= texture(gPosition, gl_FragCoord.xy).a)
	//{
		if(bAlbedo)
    		FragColor = texture(Diffuse, vTex);
    	else
    		FragColor = vec4(diffuse, 1.0);
	//}
}