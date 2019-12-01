#version 330 core
in vec2 vTex; // 后处理时的光栅化坐标

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gBackPosition; // 使用 cull_mode::front 剔除正面, 渲染背面得到的背面坐标, 主要用来结合正面坐标计算物体的厚度

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform mat4 Proj;

uniform	float farPlane;
//uniform	float nearPlane;
uniform	vec2 screenSize;

//@
/*
layout (std140) uniform light 
{
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};

layout (std140) uniform Matrix
{
	mat4 World;
	mat4 View;
	mat4 Proj;

	mat4 WorldInvTranspose;

	//vec3 cameraPos;
	//float pad1;

	//vec4 dirLightColor;

	//vec3 dirLightDir;
	//float pad2;

	//vec3 ambientLight;
	//float pad3;
};
 
layout (std140) uniform SSR
{
	float farPlane;
	float nearPlane;
	vec2 perspectiveValues;
};
*/

const int MAX_STEPS = 500;
 
//转换为屏幕空间坐标
vec2 NDCToScreenCoord(vec2 ndc)
{
	vec2 screenCoord;
	screenCoord.x = screenSize.x * (0.5 * ndc.x + 0.5); // [-1, 1] -> [0, 1] -> [0, width]
	screenCoord.y = screenSize.y * (-0.5 * ndc.y + 0.5);
	return screenCoord;
}
 
float distanceSquared(vec2 a, vec2 b)
{
	a -= b;
	return dot(a, a);
}

float distanceSquared3(vec3 a, vec3 b)
{
	a -= b;
	return dot(a, a);
}

void main()
{             
    // retrieve data from gbuffer
    vec3 cameraPos    = texture(gPosition, vTex).rgb;
    vec3 cameraNormal = texture(gNormal, vTex).rgb;
    vec3 Diffuse   = texture(gAlbedoSpec, vTex).rgb;
    float Specular = texture(gAlbedoSpec, vTex).a;
    
    vec3 viewDir  = normalize(viewPos - cameraPos);

    // ambient
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component

    // diffuse
    vec3 lightDir = normalize(lightPosition - cameraPos);
    vec3 diffuse = max(dot(cameraNormal, lightDir), 0.0) * Diffuse * lightColor;
    // specular
    //vec3 halfwayDir = normalize(lightDir + viewDir);  
    //float spec = pow(max(dot(cameraNormal, halfwayDir), 0.0), 16.0);
    //vec3 specular = lightColor * spec * Specular;
    //specular = vec3(0.0);
    // attenuation
    //float distance = length(lightPosition - cameraPos);
    //float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);

    //diffuse *= attenuation;
    //specular *= attenuation;
    lighting += diffuse;   

    lighting *= 1.3;
    FragColor = vec4(lighting, 1.0);

    if(distanceSquared3(cameraNormal, vec3(0.0, 1.0, 0.0)) > 0.1)
        return;



    vec4 reflectColor = vec4(0.0, 0.0, 0.0, 0.0);

	//ivec2 RasterOrigin__ = vTex * screenSize; // [0, 1]^2 -> [0, width] * [0, height]



    //像素在相机空间的位置(即光线起点)
	vec3 cameraOrigin = cameraPos;
 
	//相机到像素的方向
	vec3 eyeToCameraOrigin = normalize(cameraOrigin); // cameraOrigin - (0, 0, 0)
 
	//光线反射的方向
	vec3 reflectRay = normalize(reflect(eyeToCameraOrigin, cameraNormal));
 
	//反射光线终点
	vec3 cameraEnd = cameraOrigin + reflectRay * farPlane;
 


	//屏幕空间的坐标
	vec4 RasterOrigin = Proj * vec4(cameraOrigin, 1.0);
	vec4 RasterEnd = Proj * vec4(cameraEnd, 1.0);
	
	float invWOrigin = 1.0 / RasterOrigin.w;
	float invWEnd = 1.0 / RasterEnd.w;
 
	//透视校正
	cameraOrigin *= invWOrigin;
	cameraEnd *= invWEnd;

	RasterOrigin *= invWOrigin;
	RasterEnd *= invWEnd;
 
	// 转换到屏幕空间
	RasterOrigin.xy = NDCToScreenCoord(RasterOrigin.xy);
	RasterEnd.xy = NDCToScreenCoord(RasterEnd.xy);
 


	//保证屏幕空间的光线起点终点至少一个单位长度
	float rasterDist = distanceSquared(RasterEnd.xy, RasterOrigin.xy);
	RasterEnd += rasterDist < 0.0001 ? 0.01 : 0.0;
	float RasterLength = length(RasterEnd.xy - RasterOrigin.xy);
	


	vec3 deltaCamera = (cameraEnd - cameraOrigin) / RasterLength;
	float deltaInvW = (invWEnd - invWOrigin) / RasterLength;

	vec2 traceDir = ((RasterEnd - RasterOrigin) / RasterLength).xy;
 


    
	// 从 RasterOrigin 步进到 RasterEnd
	float maxSteps = min(RasterLength, MAX_STEPS);
    float t = 1;
	ivec2 coord;
    while (t < maxSteps)
	{
		// 在屏幕空间移动，保证每次至少移动一格
		coord = ivec2(RasterOrigin.xy + traceDir * t);

		if (coord.x > screenSize.x || coord.y > screenSize.y || coord.x < 0 || coord.y < 0)
		{
			break;
		}
 
		float curDepth = (cameraOrigin + deltaCamera * t).z;

		float invW = invWOrigin + deltaInvW * t; // 对 invW 进行插值
		curDepth /= invW; // 转换回相机空间

		vec2 texcoord = vec2(coord) / screenSize; // -> [0, 1]^2

		float FrontDepth = texture(gPosition, texcoord).z; // gPosition
		float BackDepth = texture(gBackPosition, texcoord).z;

		if ((curDepth > FrontDepth) && ((curDepth - FrontDepth) <= (BackDepth - FrontDepth)))
		{
			reflectColor.rgb = texture(gAlbedoSpec, texcoord).rgb;	
			break;
		}

		t++;
	}



    FragColor = FragColor * 0.4 + reflectColor * 0.6;
}
