#version 330 core
out vec4 FragColor;

in VS_OUT 
{
    vec3 wPos;
    vec3 wNor;
    vec2 wTex;
    // 以下三个量都在切线空间
    vec3 tLightPos;
    vec3 tViewPos;
    vec3 tPos;
} fs_in;

uniform sampler2D Ambient;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Parallax;

uniform vec3 LightPos;
uniform vec3 ViewPos;

uniform bool use_normal_map;
uniform float sa;
uniform float sd;
uniform float ss;
uniform float shininess;


void main()
{   
    // ambient
    vec3 ambient = 1.0 * texture(Ambient, fs_in.wTex).rgb;
    vec3 diffuse, specular;
    if(use_normal_map)
    {
        // obtain normal from normal map in range [0,1]
        vec3 normal = texture(Parallax, fs_in.wTex).rgb;
        // transform normal vector to range [-1,1]
        normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

        vec3 lightDir = normalize(fs_in.tLightPos - fs_in.tPos);
        float diff = max(dot(lightDir, normal), 0.0);
        diffuse = diff * texture(Diffuse, fs_in.wTex).rgb;

        vec3 viewDir = normalize(fs_in.tViewPos - fs_in.tPos);
        //vec3 reflfectDir = reflect(-lightDir, normal);
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
        specular = vec3(0.2) * spec;
    }
    else
    {
        vec3 normal = normalize(fs_in.wNor);

        vec3 lightdir = normalize(LightPos - fs_in.wPos);
        float kd = max(dot(normal, lightdir), 0.0);
        diffuse = kd * texture(Diffuse, fs_in.wTex).rgb;

        // spec
        vec3 viewdir = normalize(ViewPos - fs_in.wPos);
        vec3 halfwaydir, reflectdir;
        float ks;

        halfwaydir = normalize(lightdir + viewdir);
        ks = pow(max(dot(normal, halfwaydir), 0.0), shininess);
        specular = ks * texture(Specular, fs_in.wTex).rgb;
    }

    FragColor = vec4(ambient * sa + diffuse * sd + specular * ss, 1.0);
}