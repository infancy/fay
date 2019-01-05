#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RESOURCE_LOAD_MODEL_H
#define FAY_RESOURCE_LOAD_MODEL_H

// TODO: #include "fay/math/geometry.h"
// TODO: add : model info, save (sub)model
#include "fay/gl/geometry.h"
#include "fay/resource/image.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace fay
{

// model_loader, scene_loader

// template<tynename vertex>
template<typename Vertex>
struct resource_mesh
{
	std::vector<Vertex>   vertices;
	std::vector<uint32_t> indices;
	std::vector<std::pair<image, texture_format>> images;
};

struct resource_model
{
	const std::string path;	// resources_directory
	render_backend_type api;

	// glm::vec3 min{}, max{};

	resource_model(const std::string& filepath, render_backend_type api) :
        path{ get_directory(filepath) }, api{ api } {}
};

// load model by fay -----------------------------------------------------------

enum class obj_keyword
{
	comment, // '#'
	v, vn, vt, 
	o, g, s, f,
	mtllib, usemtl, newmtl,
	Ns, Ni, d, Tr, Tf, illum,
	Ka, Kd, Ks, Ke,
	map_Ka, map_Kd, map_Ks, map_Ke, map_d, map_bump
};

struct obj_mesh
{
	std::string name{};
	std::string mat_name{};
	int smoothing_group{};

	std::vector<vertex3>  vertices{};
	std::vector<uint32_t> indices{};
};

struct obj_material
{
	std::string name{};

	// glm::vec3 ambient{};
	// glm::vec3 diffuse{};
	// glm::vec3 specular{};

	float Ns, Ni, d, Tr, Tf;
	int illum;
	glm::vec3 Ka{};
	glm::vec3 Kd{};
	glm::vec3 Ks{};
	glm::vec3 Ke{};

	// static constexpr int nMap = 6;
	std::string map_Ka, map_Kd, map_Ks, map_Ke, map_d, map_bump;
	// std::vector<uint32_t> sub_indices;
	// int offset;
	// int count;

	// obj_material() { memset(this, 0, sizeof(obj_material)); }
};

class obj_model : public resource_model
{
public:
	obj_model(const std::string& filepath, render_backend_type api = render_backend_type::opengl);

	// TODO: bound3
	std::pair<glm::vec3, glm::vec3> bbox();	// 分离出 bbox 以：不为不需要的东西付出代价 & 避免复杂的加载代码

private:
	std::vector<obj_mesh> load_meshs(const std::string& firstline, std::ifstream& file);
	// 也可以在 load_materials() 里直接构造 materials，但这样写更清楚
	std::unordered_map<std::string, obj_material> 
		load_materials(const std::string& filepath);

public:
	std::vector<std::pair<obj_mesh, obj_material>> meshes;
};

// std::vector<mesh> create_meshes(const obj_model& obj);

// load model by assimp --------------------------------------------------------

using assimp_mesh = resource_mesh<vertex5>;

class assimp_model : public resource_model
{
public:
	assimp_model(const std::string& filepath, render_backend_type api = render_backend_type::opengl, model_format model_format = model_format::obj);

private:
	void process_node(aiNode* node, const aiScene* scene);
	assimp_mesh process_mesh(aiMesh* mesh, const aiScene* scene);

	std::vector<std::pair<image, texture_format>> 
	load_images(aiMaterial* mat, aiTextureType type, texture_format textype);

public:
	std::vector<assimp_mesh> meshes;

private:
	model_format modeltype;
	std::unordered_map<std::string, image> images_cache;	// 保存已加载的图像，避免重复加载
};

} // namespace fay

#endif // FAY_RESOURCE_LOAD_MODEL_H