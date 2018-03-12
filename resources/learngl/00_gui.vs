#version 330 core 
  
layout(location = 0) in vec3 position; // 渲染覆盖整个视口的长方形

smooth out vec2 uv;

uniform mat4 MVP;					
 
void main()
{  
	uv = position.xy;	
	gl_Position = MVP * vec4(position.xyz, 1);
}