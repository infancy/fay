#version 330 core
in vec2 vTex;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light 
{
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos   = texture(gPosition, vTex).rgb;
    vec3 Normal    = texture(gNormal, vTex).rgb;
    vec3 Diffuse   = texture(gAlbedoSpec, vTex).rgb;
    float Specular = texture(gAlbedoSpec, vTex).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);

    FragColor = vec4(Diffuse, 1.0);

    //FragColor = texture(gAlbedoSpec, vTex);
}
