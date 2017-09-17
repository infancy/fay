#version 330 core 
  
layout(location = 0) in vec3 vVertex; //object space vertex position

//output to fragment shader
smooth out vec2 vUV;					
 
void main()
{  
	//set the current object space position as the output texture coordinates
	//and the clip space position
	vUV = vVertex.xy;	
	gl_Position = vec4(vVertex.xyz, 1);
}