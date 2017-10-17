#version 330 core
 
layout(location = 0) in vec3 vVertex;	
layout(location = 1) in vec3 vNormal;	
layout(location = 2) in vec2 vUV;		
 
// 变换矩阵
uniform mat4 P; 
uniform mat4 MV;
uniform mat3 N;

smooth out vec2 vUVout;					
smooth out vec3 vEyeSpaceNormal;    	// 相机空间
smooth out vec3 vEyeSpacePosition;		

void main()
{
	vUVout=vUV; 
 
	vEyeSpacePosition = (MV*vec4(vVertex,1)).xyz; 

	vEyeSpaceNormal   = N*vNormal;

	gl_Position = P*vec4(vEyeSpacePosition,1);
}