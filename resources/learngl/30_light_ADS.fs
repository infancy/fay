#version 330 core
in vec3 vPos;
in vec3 vNor;
in vec2 vTex;

out vec4 FragColor;

uniform sampler2D diffuse;
uniform sampler2D specular;

uniform vec3 vLightPos;
uniform vec3 lightcolor;

// strength
uniform float sa;
uniform float sd;
uniform float ss;
uniform int shininess;

void main()
{    
    // ambient
    float ka = 1.0;
    vec4 ambi = ka * sa * texture(diffuse, vTex) * vec4(lightcolor, 1.0);

    // diff
    vec3 normal = normalize(vNor);
    vec3 lightdir = normalize(vLightPos - vPos);
    float kd = max(dot(normal, lightdir), 0.0);
    vec4 diff = kd * sd * texture(diffuse, vTex) * vec4(lightcolor, 1.0);

    // spec
    vec3 viewdir = normalize(-vPos);
    vec3 reflectdir = reflect(-lightdir, normal);
    float ks = pow(max(dot(viewdir, reflectdir), 0.0), shininess);
    vec4 spec = ks * ss * texture(specular, vTex) * vec4(lightcolor, 1.0);

    FragColor = ambi + diff + spec;
}