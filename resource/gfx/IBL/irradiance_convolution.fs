#version 330 core
in VS_OUT 
{
    vec3 wPos0;
    vec3 wPos1;
    vec3 wPos2;
    vec3 wPos3;
    vec3 wPos4;
    vec3 wPos5;
} fs_in;

layout (location = 0) out vec3 FragColor0;
layout (location = 1) out vec3 FragColor1;
layout (location = 2) out vec3 FragColor2;
layout (location = 3) out vec3 FragColor3;
layout (location = 4) out vec3 FragColor4;
layout (location = 5) out vec3 FragColor5;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

vec3 SampleEnvironmentMap(vec3 wPos)
{	
    // return texture(environmentMap, wPos).rgb;

	// The world vector acts as the normal of a tangent surface
    // from the origin, aligned to wPos. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    vec3 N = normalize(wPos);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, N);
    up         = cross(N, right);
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 在这里的切线空间中把 right 当作 x 轴，up 当作 y 轴，normal 当作 z 轴
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            ++nrSamples;
        }
    }

    return PI * irradiance * (1.0 / float(nrSamples));
}

void main()
{
    FragColor0 = SampleEnvironmentMap(fs_in.wPos0);
    FragColor1 = SampleEnvironmentMap(fs_in.wPos1);
    
    // negative y
    FragColor2 = SampleEnvironmentMap(vec3(-fs_in.wPos2.z, fs_in.wPos2.y, fs_in.wPos2.x)); // turn 90 degrees clockwise
    // positive y
    FragColor3 = SampleEnvironmentMap(vec3(-fs_in.wPos3.z, fs_in.wPos3.y, fs_in.wPos3.x)); // turn 90 degrees inverse clockwise 

    FragColor4 = SampleEnvironmentMap(fs_in.wPos4);
    FragColor5 = SampleEnvironmentMap(fs_in.wPos5);
}
