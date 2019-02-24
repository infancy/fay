#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;

uniform mat4 proj;
uniform mat4 view;

out vec3 wPos;

void main()
{
    wPos = mPos;

	mat4 rotView = mat4(mat3(view)); // set the camera to center
	vec4 clipPos = proj * rotView * vec4(wPos, 1.0);

	gl_Position = clipPos.xyww; // make z always is 1
}