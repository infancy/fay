#version 330 core
out vec4 FragColor;

in VS_OUT 
{
    vec3 wPos;
    vec3 wNor;
    vec2 wTex;
    vec4 LightSpacePos; // Light NDC Space
} fs_in;

uniform sampler2D Diffuse;
uniform sampler2D Shadowmap;

uniform vec3 LightPos;
uniform vec3 ViewPos;

float ShadowCalculation(vec4 fragPosLightSpace) // , i)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(Shadowmap, projCoords.xy).r; 
    // 取得当前片元在光源视角下的深度
    float currentDepth = projCoords.z;

    // 检查当前片元是否在阴影中
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float bias = 0.005;

    if(currentDepth < closestDepth)
        return 0.0;
    else
        return 1.0;
    /*
    if(projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(Shadowmap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(Shadowmap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }

    return shadow / 9.0;
    */
}

void main()
{           
    vec3 color = texture(Diffuse, fs_in.wTex).rgb;
    vec3 normal = normalize(fs_in.wNor);
    vec3 lightColor = vec3(1.0);
    // Ambient
    vec3 ambient = 0.05 * color;
    // Diffuse
    vec3 lightDir = normalize(LightPos - fs_in.wPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // Specular
    vec3 viewDir = normalize(ViewPos - fs_in.wPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // 计算阴影
    float shadow = ShadowCalculation(fs_in.LightSpacePos);       
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    

    FragColor = vec4(lighting, 1.0f);
}