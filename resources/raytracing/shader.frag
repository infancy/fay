#version 330 core
  
layout(location=0) out vec4 vFragColor;

// uniforms
uniform sampler2D texture_diffuse1;
uniform mat4 MV;			
uniform vec3 light_position;		

smooth in vec3 vEyeSpaceNormal;	   
smooth in vec3 vEyeSpacePosition;	
smooth in vec2 vUVout;				

// 距离衰减
const float k0 = 1.0;	
const float k1 = 0.0;	
const float k2 = 0.0;	

void main()
{  
	vec4 vEyeSpaceLightPos = MV*vec4(light_position,1);

	vec3 L = (vEyeSpaceLightPos.xyz-vEyeSpacePosition);

	float d = length(L);

	L = normalize(L);

	float diffuse = max(0, dot(vEyeSpaceNormal, L));	

	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;

    vFragColor =  diffuse * texture(texture_diffuse1, vUVout);
}