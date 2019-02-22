#version 330 core
layout (location = 0) in vec3 mPos;
layout (location = 1) in vec3 mNor;
layout (location = 2) in vec2 mTex;
layout (location = 3) in vec3 mTan;
layout (location = 4) in vec3 mBit;
layout (location = 5) in mat4 iModel; // instance data, 4 * vec4

out VS_OUT 
{
    vec3 wPos;
    vec3 wNor;
    vec2 wTex;
    vec4 CameraSpacePos;
} vs_out;

uniform mat4 Proj;
uniform mat4 View;
uniform mat4 Model;

void main()
{
    mat4 ModelMat = Model;

    gl_Position = Proj * View * ModelMat * vec4(mPos, 1.0f);
    // 在世界空间中做计算
    vs_out.wPos = vec3(ModelMat * vec4(mPos, 1.0));
    vs_out.wNor = transpose(inverse(mat3(ModelMat))) * mNor;
    vs_out.wTex = mTex;

    vs_out.CameraSpacePos = View * ModelMat * vec4(mPos, 1.f);
}