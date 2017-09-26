#version 330 core
layout (location = 0) in vec3 aPos;
uniform vec4 position;
//out vec3 ourColor;
void main()
{
	//gl_PointSize = 100.0;
	//gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	//gl_Position = position;
	gl_Position = vec4(aPos.x + position.x, aPos.y - position.y, 0.0, 1.0);
}