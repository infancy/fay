#version 330 core
out vec4 FragColor;
in vec3 wPos;

uniform samplerCube environmentMap;

void main()
{		
    // WARNNING: -y
    vec3 envColor = texture(environmentMap, vec3(wPos.x, -wPos.y, wPos.z)).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(envColor, 1.0);
}
