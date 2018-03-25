#version 330 core
in vec2 vTex;
out vec4 FragColor;

float invgamma = 0.454545;  // 1.0 / 2.2

uniform bool gamma_correction;
uniform sampler2D Diffuse;

void main()
{    
    FragColor = texture(Diffuse, vTex);
    //FragColor.g =  FragColor.r;
    //FragColor.b =  FragColor.r;

    if(gamma_correction)
        FragColor.rgb = pow(FragColor.rgb, vec3(invgamma));
}