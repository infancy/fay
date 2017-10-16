#version 330 core

layout(location = 0) out vec4 vFragColor; // 输出颜色

struct Ray { vec3 origin, dir;} eyeRay; 
struct Box { vec3 min, max; };

smooth in vec2 vUV;		// 平滑插值得到的光栅化坐标

// uniforms
uniform int samples_PerPixel;		// 每像素采样次数
uniform mat4 invMV;				    // MV 矩阵的逆矩阵
uniform vec4 backgroundColor;		
uniform vec3 eyePos;				// 对象空间中的视点
uniform sampler2D vertex_positions;	// 顶点
uniform isampler2D triangles_list;	// 索引
uniform sampler2DArray textureMaps;	// 纹理
uniform vec3 light_position;		
uniform Box aabb;					
uniform float VERTEX_TEXTURE_SIZE;	 // 顶点数量
uniform float TRIANGLE_TEXTURE_SIZE; // 索引数量
 
// 距离衰减
const float k0 = 1.0;	// 常数项
const float k1 = 0.0;	// 线性项
const float k2 = 0.0;	// 平方项
 
// ray-box intersection
//returns a vec2 in which the x value contains the t value at the near intersection
						//the y value contains the t value at the far intersection
vec2 intersectCube(vec3 origin, vec3 ray, Box cube) 
{		
	vec3   tMin = (cube.min - origin) / ray;		
	vec3   tMax = (cube.max - origin) / ray;		
	vec3     t1 = min(tMin, tMax);		
	vec3     t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float  tFar = min(min(t2.x, t2.y), t2.z);
	return vec2(tNear, tFar);	
}

// 生成相机光线
void setup_camera(vec2 uv) 
{
  eyeRay.origin = eyePos; 

  // 1080/720 = 1.5；-1 是因为 near = 1，且相机坐标是右手系的；这一步的作用是 NDC_to_screen_to_camera
  vec4 cam_dir = vec4(uv.x * 1.5, uv.y * 1, -1, 0);
  eyeRay.dir = (invMV * cam_dir).xyz;	// invMV, camera_to_world
}

// 生成伪随机数
float random(vec3 scale, float seed) 
{		
	return fract(sin(dot(gl_FragCoord.xyz + seed, scale)) * 43758.5453 + seed);	
}	

// 生成一个随机方向
vec3 uniformlyRandomDirection(float seed) 
{		
	float u = random(vec3(12.9898, 78.233, 151.7182), seed);		
	float v = random(vec3(63.7264, 10.873, 623.6736), seed);		
	float z = 1.0 - 2.0 * u;		
	float r = sqrt(1.0 - z * z);	
	float angle = 6.283185307179586 * v;	
	return vec3(r * cos(angle), r * sin(angle), z);	
}	

vec3 uniformlyRandomVector(float seed) 
{		
	return uniformlyRandomDirection(seed) * sqrt(random(vec3(36.7539, 50.3658, 306.2759), seed));	
}	

// ray-triangle intesection 
// The return value is a vec4 with
// x -> t value at intersection.
// y -> u texture coordinate
// z -> v texture coordinate
// w -> texture map id
vec4 intersectTriangle(vec3 origin, vec3 dir, int index,  out vec3 normal ) 
{
	ivec4 list_pos = texture(triangles_list, vec2((index+0.5)/TRIANGLE_TEXTURE_SIZE, 0.5));
	if((index+1) % 2 !=0 ) { 
		list_pos.xyz = list_pos.zxy;
	}  
	vec3 v0 = texture(vertex_positions, vec2((list_pos.z + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	vec3 v1 = texture(vertex_positions, vec2((list_pos.y + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	vec3 v2 = texture(vertex_positions, vec2((list_pos.x + 0.5 )/VERTEX_TEXTURE_SIZE, 0.5)).xyz;
	  
	vec3 e1 = v1-v0;
	vec3 e2 = v2-v0;
	vec3 tvec = origin - v0;  
	
	vec3 pvec = cross(dir, e2);  
	float  det  = dot(e1, pvec);   

	float inv_det = 1.0/ det;  

	float u = dot(tvec, pvec) * inv_det;  

	if (u < 0.0 || u > 1.0)  
		return vec4(-1,0,0,0);  

	vec3 qvec = cross(tvec, e1);  

	float v = dot(dir, qvec) * inv_det;  

	if (v < 0.0 || (u + v) > 1.0)  
		return vec4(-1,0,0,0);  

	float t = dot(e2, qvec) * inv_det;
	if((index+1) % 2 ==0 ) {
		v = 1-v; 
	} else {
		u = 1-u;
	} 

	normal = normalize(cross(e2,e1));
	return vec4(t,u,v,list_pos.w);
}

float shadow(vec3 origin, vec3 dir ) 
{
	vec3 tmp;
	for(int i = 0; i < int(TRIANGLE_TEXTURE_SIZE); i++) 
	{
		vec4 res = intersectTriangle(origin, dir, i, tmp); 
		if(res.x > 0 ) 
		{ 
		   return 0.5;   
		}
	}
	return 1.0;
}

vec4 rayCast(vec2 uv)
{ 
	// 最大距离
	float t = 10000;  

	// 根据给定的在光栅化坐标生成相机
	setup_camera(uv);
	 
	// 检查是否超出场景包围盒
	vec2 tNearFar = intersectCube(eyeRay.origin, eyeRay.dir, aabb);

	// if near intersection < far intersection
	if(tNearFar.x < tNearFar.y  ) {
		
		t = tNearFar.y+1; // 对 far intersection 向后偏置，以防止失真
		  
		// 三角形求交测试
		 
		vec4 val=vec4(t,0,0,0);
		vec3 N;
		// 没有使用加速结构，因而比较低效
		for(int i=0; i < int(TRIANGLE_TEXTURE_SIZE); i++) 
		{
			vec3 normal;
			vec4 res = intersectTriangle(eyeRay.origin, eyeRay.dir, i, normal); 
		 	if(res.x>0 && res.x <= val.x) {
			   val = res;  
			   N = normal;
		    }
		}

		if(val.x < t) {			 
			// 计算交点位置
			vec3 hit = eyeRay.origin + eyeRay.dir*val.x;

			// 从光源处随机投射一条光线
			vec3  jitteredLight  =  light_position +  uniformlyRandomVector(gl_FragCoord.x);			
		
			// 计算从交点到光源的向量
			vec3 L = (jitteredLight.xyz-hit);
			float d = length(L);
			L = normalize(L);

			// 计算漫反射项
			float diffuse = max(0, dot(N, L));	

			// 应用基于距离的衰减
			float attenuationAmount = 1.0/(k0 + (k1*d) + (k2*d*d));
			diffuse *= attenuationAmount;

			// 是否处于阴影中
			float inShadow = shadow(hit+ N*0.0001, L) ;

			return inShadow*diffuse*mix(texture(textureMaps, val.yzw), vec4(1), (val.w==255) );
		}    
	} 
	return backgroundColor;
}

void main()
{
	vec4 color = vec4(0, 0, 0, 0);
	for(int i = 0; i < samples_PerPixel; ++i)
	{
		color += rayCast(vUV);
	}

	vFragColor = color / samples_PerPixel;
	return;
}


