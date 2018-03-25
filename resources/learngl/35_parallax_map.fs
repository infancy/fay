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

uniform sampler2D Diffuse;
uniform sampler2D Normal;
// uniform sampler2D Height;
uniform sampler2D Depth;
//uniform sampler2D Parallax;

uniform vec3 LightPos;
uniform vec3 ViewPos;

uniform bool use_normal_map;
uniform float sa;
uniform float sd;
uniform float ss;
uniform float shininess;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    float height =  texture(Depth, texCoords).r;     
    return texCoords - viewDir.xy * (height * heightScale);        
}

void main()
{           
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.tViewPos - fs_in.tPos);
    //vec2 texCoords = fs_in.wTex;
    vec2 texCoords = ParallaxMapping(fs_in.wTex,  viewDir);       
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    // obtain normal from normal map
    vec3 normal = texture(Normal, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
   
    // get diffuse color
    vec3 color = texture(Diffuse, texCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.tLightPos - fs_in.tPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient * sa + diffuse * sd + specular * ss, 1.0);
}