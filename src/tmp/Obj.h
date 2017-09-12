#ifndef OBJ_INC
#define OBJ_INC
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

using namespace std;

namespace tmp
{

struct BBox { glm::vec3 min, max; };

struct Vertex	{  
	glm::vec3 pos, 
			  normal;
	glm::vec2 uv;
}; 

struct Face { 
	unsigned short	a,b,c,  //pos indices
					d,e,f,  //normal indices
					g,h,i;  //uv indices
};

class  Mesh { 
	public:
		Mesh() {
			material_index=-1;
			
		}
		~Mesh(){}
		string			name;
		int				material_index;
};

class Material {
public:
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float Tf[3];
	int illum;
	float Ka[3];
	float Kd[3];
	float Ks[3];
	float Ke[3];
	std::string map_Ka,  map_Kd, name; // name 是材质名
	float Ns, Ni, d, Tr; 
	vector<unsigned short> sub_indices;	// 将索引先存到材质中，当材质多于一个时，从材质渲染网格
	int offset;
	int count;

};

bool Load(const string& filename, vector<Mesh*>& meshes, vector<Vertex>& verts, vector<unsigned short>& inds,	vector<Material*>& materials, BBox& aabb, vector<glm::vec3>& verts2, vector<unsigned short>& inds2);	

}
#endif
