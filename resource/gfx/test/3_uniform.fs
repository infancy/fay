#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D Diffuse;

layout (std140) uniform color
{
    vec4 a;
    vec4 b;
};

uniform int flag;

void main()
{
   if(flag == 1)
       FragColor = texture(Diffuse, vTex);
   else
       FragColor = texture(Diffuse, vTex);

   //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}