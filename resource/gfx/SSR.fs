#version 330 core
in vec2 vTex;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gFrontBackDepth;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 viewPos;
/*
layout (std140) uniform light 
{
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};
*/


void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos   = texture(gPosition, vTex).rgb;
    vec3 Normal    = texture(gNormal, vTex).rgb;
    vec3 Diffuse   = texture(gAlbedoSpec, vTex).rgb;
    float Specular = texture(gAlbedoSpec, vTex).a;
    
    vec3 viewDir  = normalize(viewPos - FragPos);

    // ambient
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component

    // diffuse
    vec3 lightDir = normalize(lightPosition - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColor;
    // specular
    //vec3 halfwayDir = normalize(lightDir + viewDir);  
    //float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    //vec3 specular = lightColor * spec * Specular;
    //specular = vec3(0.0);
    // attenuation
    //float distance = length(lightPosition - FragPos);
    //float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);

    //diffuse *= attenuation;
    //specular *= attenuation;
    lighting += diffuse;   

    lighting *= 1.3;
    FragColor = vec4(lighting, 1.0);
}
