#version 330 core
in vec2 vTex;

uniform sampler2D uAccumulate;
uniform sampler2D uAccumulateAlpha;

out vec4 FragColor;

void main() 
{
    vec4 accum = texture(uAccumulate, vTex);
    float a = 1.0 - accum.a;
    accum.a = texture(uAccumulateAlpha, vTex).r;

    FragColor = vec4(a * accum.rgb / clamp(accum.a, 0.001, 50000.0), a);
}