#version 330 core

smooth in vec2 uv;		// 平滑插值得到的 UV 坐标

layout(location = 0) out vec4 FragColor; 

uniform sampler2D diff;

void main()
{    
    FragColor = texture(diff, uv);
}