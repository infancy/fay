#version 330 core

layout(location = 0) out vec4 vFragColor;

struct Ray { vec3 origin, dir;} eyeRay; 
struct Box { vec3 min, max; };

smooth in vec2 vUV;		

// uniforms
uniform int samples_PerPixel;		
uniform mat4 invMV;					
uniform vec4 backgroundColor;			
uniform vec3 eyePos; 					
uniform sampler2D vertex_positions;		
uniform isampler2D triangles_list;		
uniform sampler2DArray textureMaps;		
uniform vec3 light_position;			
uniform Box aabb;	 					
uniform float VERTEX_TEXTURE_SIZE; 		
uniform float TRIANGLE_TEXTURE_SIZE; 	 
uniform float time;						// 当前时间

// constants
const int MAX_BOUNCES = 3;	// 最大深度

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

// ray-triangle intesection 
// The return value is a vec4 with
// x -> t value at intersection.
// y -> u texture coordinate
// z -> v texture coordinate
// w -> texture map id 
vec4 intersectTriangle(vec3 origin, vec3 dir, int index, out vec3 normal ) 
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
	return uniformlyRandomDirection(seed) *  (random(vec3(36.7539, 50.3658, 306.2759), seed));	
}

float shadow(vec3 origin, vec3 dir ) 
{
	vec3 tmp;
	for(int i=0;i<int(TRIANGLE_TEXTURE_SIZE);i++) 
	{
		vec4 res = intersectTriangle(origin, dir, i, tmp); 
		if(res.x>0 ) { 
		   return 0.5;   
		}
	}
	return 1.0;
}

vec3 pathtrace_impl(vec3 origin, vec3 ray, vec3 light, float t) 
{		
	vec3 colorMask = vec3(1.0);		
	vec3 accumulatedColor = vec3(0.0);
	vec3 surfaceColor=vec3(backgroundColor.xyz);
	
	float diffuse = 1;

	for(int bounce = 0; bounce < MAX_BOUNCES; bounce++) 
	{			
		vec2 tNearFar = intersectCube(origin, ray,  aabb);
		
		// 是否在包围盒内
		if(  tNearFar.x > tNearFar.y)  
		   continue; 
		
		//if it nearest so far set the t paramter
		if(tNearFar.y<t)
			t =   tNearFar.y+1;					
		
		vec3 N;
		vec4 val=vec4(t,0,0,0); 

		// 三角形求交，没有使用加速结构，因而比较低效
		for(int i=0;i<int(TRIANGLE_TEXTURE_SIZE);i++) 
		{
			vec3 normal;
			vec4 res = intersectTriangle(origin, ray, i, normal); 
			// 避免椒盐噪声
		 	if(res.x>0.001 && res.x <  val.x) 
			 { 
			   val = res;   // 更新最近交点
			   N = normal;
		    }
		}
		   
		if(  val.x < t)
		{			  	
			surfaceColor = mix(texture(textureMaps, val.yzw), vec4(1), (val.w==255) ).xyz; 
			
			// 发射新的光线
			vec3 hit = origin + ray * val.x;	
			origin = hit;	
			ray = uniformlyRandomDirection(time + float(bounce));	
			
			// 对光源进行一定的偏置以避免失真
			vec3  jitteredLight  =  light + ray;
			vec3 L = normalize(jitteredLight - hit);			
			
			diffuse = max(0.0, dot(L, N ));			 		
			colorMask *= surfaceColor;			

			float inShadow = shadow(hit+ N*0.0001, L);

			// 累计颜色
			accumulatedColor += colorMask * diffuse * inShadow; 
			t = val.x;
		}  			
	}		
	// 如果 accumulatedColor 为 0，则返回背景颜色
	if(accumulatedColor == vec3(0))
		return surfaceColor*diffuse;
	else
		return accumulatedColor/float(MAX_BOUNCES-1);	
}	

vec4 pathtrace(vec2 uv)
{ 
	// 最大距离
	float t = 10000;  

	setup_camera(uv);
	
	// 检查是否超出场景包围盒
	vec2 tNearFar = intersectCube(eyeRay.origin, eyeRay.dir,  aabb);

	// if near intersection < far intersection
	if(tNearFar.x<tNearFar.y  ) 
	{
		t = tNearFar.y+1; // 对 far intersection 向后偏置，以防止失真
		  		 
		// 跟踪一条从光源开始某一方向发射的随机光线		
		vec3 light = light_position + uniformlyRandomVector(time);
 
		return vec4(pathtrace_impl(eyeRay.origin, eyeRay.dir, light, t),1);		 
	} 
	return backgroundColor;
}

void main()
{
	vec4 color = vec4(0, 0, 0, 0);
	for(int i = 0; i < samples_PerPixel; ++i)
	{
		float u = random(vec3(12.9898, 78.233, 151.7182), gl_FragCoord.x + i);		
		float v = random(vec3(63.7264, 10.873, 623.6736), gl_FragCoord.y + i);
		vec2 UV = vUV + vec2((2*u - 0.5) / 1080, (2*v - 0.5) / 720);
		color += pathtrace(UV);
	}

	vFragColor = color / samples_PerPixel;
	return;
}
