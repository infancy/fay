#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D Diffuse;

layout (std140) uniform para
{
    vec4 window;
    int flag;
};

//uniform int flag;
//uniform vec4 window; // origin_xy, width, height

void main()
{
    if(flag == 1)
        FragColor = texture(Diffuse, vTex);
    else
    {
        float y = gl_FragCoord.y / window.w;
        FragColor = vec4(y, y, y, 1.f);
    }

    if((gl_FragCoord.x < window.z / 2) && (gl_FragCoord.y < window.w / 2))
        FragColor = vec4(1.0, 0.0, 0.0, 1.f);
}