#version 330 core

in vec3 vPos;
in vec2 vTex;
in vec3 vNor;

uniform vec4 uLightPosition;
uniform sampler2D Albedo;

layout(location=0) out vec4 accumColor;
layout(location=1) out vec4 accumAlpha;

float weight(float z, float a) 
{
    return clamp(pow(min(1.0, a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - z * 0.9, 3.0), 1e-2, 3e3);
}

void main() 
{
    vec3 position = vPos.xyz;
    vec3 normal = normalize(vNor.xyz);
    vec2 uv = vTex;
    vec4 baseColor = texture(Albedo, uv);

    vec3 eyeDirection = normalize(-position);
    vec3 lightVec = uLightPosition.xyz - position;
    vec3 lightDirection = normalize(lightVec);
    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float nDotL = max(dot(lightDirection, normal), 0.0);
    float diffuse = nDotL;
    float ambient = 0.2;
    float specular = pow(max(dot(reflectionDirection, eyeDirection), 0.0), 20.0);

    vec4 color = vec4((ambient + diffuse + specular) * baseColor.rgb, baseColor.a);

    color.rgb *= color.a;
    float w = weight(gl_FragCoord.z, color.a);

    accumColor = vec4(color.rgb * w, color.a);
    accumAlpha.r = color.a * w;
}