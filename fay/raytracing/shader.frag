#version 330 core
  
layout(location=0) out vec4 vFragColor; //fragment shader output

//uniforms
uniform sampler2D texture_diffuse1;
uniform mat4 MV;					//modelview matrix
uniform vec3 light_position;		//light position in object space

//inputs from the vertex shader
smooth in vec3 vEyeSpaceNormal;		//eye space normal from the vertex shader   
smooth in vec3 vEyeSpacePosition;	//eye space position from the vertex shader
smooth in vec2 vUVout;				//texture coordinates from the vertex shader

//shader constants
const float k0 = 1.0;	//constant attenuation
const float k1 = 0.0;	//linear attenuation
const float k2 = 0.0;	//quadratic attenuation

void main()
{  
	//get the eye space light position
	vec4 vEyeSpaceLightPos = MV*vec4(light_position,1);
	//get the light vector
	vec3 L = (vEyeSpaceLightPos.xyz-vEyeSpacePosition);
	//get the distance of light
	float d = length(L);
	//normalize the light vector
	L = normalize(L);
	//get the diffuse component
	float diffuse = max(0, dot(vEyeSpaceNormal, L));	
	//apply attenuation
	float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
	diffuse *= attenuationAmount;
	//return final output colour
    vFragColor =  diffuse * texture(texture_diffuse1, vUVout);
}