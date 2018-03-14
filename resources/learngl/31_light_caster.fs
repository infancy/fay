#version 330 core
in vec3 vPos;
in vec3 vNor;
in vec2 vTex;

out vec4 FragColor;

uniform sampler2D diff;
uniform sampler2D spec;

struct DirectLight
{
    vec3 direct;
    vec3 color;
};

struct PointLight
{
    vec3 pos;
    vec3 color;
    vec3 falloff;   // constant, linear, quadratic
};

struct SpotLight
{
    vec3 pos;
    vec3 direct;
    vec3 color;
    vec2 cutoff;    // cos value of inner, outer cutoff
    vec3 falloff;   // constant, linear, quadratic
};

uniform DirectLight dLight;
uniform PointLight pLight;
uniform SpotLight sLight;

// strength
uniform float sa;
uniform float sd;
uniform float ss;
uniform int shininess;

vec4 directlight(vec3 normal, vec3 viewdir)
{
    // diff
    vec3 lightdir = normalize(-dLight.direct);
    float kd = max(dot(normal, lightdir), 0.0);
    vec4 diffuse = kd * sd * texture(diff, vTex) * vec4(dLight.color, 1.0);
    // spec
    vec3 reflectdir = reflect(-lightdir, normal);
    float ks = pow(max(dot(viewdir, reflectdir), 0.0), shininess);
    vec4 specular = ks * ss * texture(spec, vTex) * vec4(dLight.color, 1.0);
    
    return diffuse + specular;
}

vec4 pointlight(vec3 normal, vec3 viewdir)
{
    // diff
    vec3 lightdir = normalize(pLight.pos - vPos);
    float kd = max(dot(normal, lightdir), 0.0);
    vec4 diffuse = kd * sd * texture(diff, vTex) * vec4(pLight.color, 1.0);
    // spec
    vec3 reflectdir = reflect(-lightdir, normal);
    float ks = pow(max(dot(viewdir, reflectdir), 0.0), shininess);
    vec4 specular = ks * ss * texture(spec, vTex) * vec4(pLight.color, 1.0);
    
    float distance = length(pLight.pos - vPos);
    float falloff = 1.0 / (pLight.falloff.x + pLight.falloff.y * distance
        +  pLight.falloff.z * (distance * distance));
    return (diffuse + specular) * falloff;
}

vec4 spotlight(vec3 normal, vec3 viewdir)
{
    // diff
    vec3 lightdir = normalize(sLight.pos - vPos);
    float kd = max(dot(normal, lightdir), 0.0);
    vec4 diffuse = kd * sd * texture(diff, vTex) * vec4(sLight.color, 1.0);
    // spec
    vec3 reflectdir = reflect(-lightdir, normal);
    float ks = pow(max(dot(viewdir, reflectdir), 0.0), shininess);
    vec4 specular = ks * ss * texture(spec, vTex) * vec4(sLight.color, 1.0);
    
    float distance = length(sLight.pos - vPos);
    float falloff = 1.0 / (sLight.falloff.x + sLight.falloff.y * distance
        +  sLight.falloff.z * (distance * distance));

    float theta     = dot(-lightdir, normalize(sLight.direct));
    float epsilon   = sLight.cutoff.x - sLight.cutoff.y;
    float intensity = clamp((theta - sLight.cutoff.y) / epsilon, 0.0, 1.0);   

    return (diffuse + specular) * falloff * intensity;
}

void main()
{    
    // ambient
    float ka = 1.0;
    vec4 ambient = ka * sa * texture(diff, vTex);

    vec3 normal = normalize(vNor);
    vec3 viewdir = normalize(-vPos);

    FragColor 
        = ambient 
        // + directlight(normal, viewdir);
        //+ pointlight(normal, viewdir);
        + spotlight(normal, viewdir);
}