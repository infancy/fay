#version 330 core
in vec3 vPos;
in vec3 vNor;
out vec4 FragColor;

uniform samplerCube Cubemap;

void main()
{             
    // reflect
    //vec3 I = normalize(vPos);
    //vec3 R = reflect(I, normalize(vNor));
    //FragColor = vec4(texture(cubemap, R).rgb, 1.0);
    // refract
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(vPos);
    vec3 R = refract(I, normalize(vNor), ratio);
    FragColor = vec4(texture(Cubemap, R).rgb, 1.0);
}