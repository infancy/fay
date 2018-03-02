#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_LOAD_MODEL_H
#define FAY_UTILITY_LOAD_MODEL_H

#include "fay/gl/resources.h"
#include "fay/utility/image.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace fay
{

// template<tynename Vertex>
template<typename Vertex>
struct BaseMesh
{
	// template<typename Vertex> 
	std::vector<Vertex>   vertices;
	std::vector<uint32_t> indices;
	std::vector<std::pair<Image, TexType>> images;

	BaseMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, 
		std::vector<std::pair<Image, TexType>>& images) :
		vertices{ vertices }, indices{ indices }, images{ images }
	{}
};

using ObjMesh    = BaseMesh<Vertex3>;
using AssimpMesh = BaseMesh<Vertex5>;

// model -----------------------------------------------------------------------

struct BaseModel
{
	const std::string path;	// resources_directory
	Thirdparty api;

	BaseModel(const std::string& filepath, Thirdparty api);
};

// load model by fay -----------------------------------------------------------

class ObjModel : public BaseModel
{
public:
	std::vector<ObjMesh> meshes;

public:
	ObjModel(const std::string& filepath, Thirdparty api = Thirdparty::gl);
};

// std::vector<Mesh> create_meshes(const ObjModel& obj);

// load model by assimp --------------------------------------------------------

class AssimpModel : public BaseModel
{
public:
	AssimpModel(const std::string& filepath, Thirdparty api = Thirdparty::gl);

private:
	void process_node(aiNode* node, const aiScene* scene);
	AssimpMesh process_mesh(aiMesh* mesh, const aiScene* scene);

	std::vector<std::pair<Image, TexType>> 
	load_images(aiMaterial* mat, aiTextureType type, TexType textype);

public:
	std::vector<AssimpMesh> meshes;

private:
	std::unordered_map<std::string, Image> images_cache;	// 保存已加载的图像，避免重复加载
};

} // namespace fay

#endif // FAY_UTILITY_LOAD_MODEL_H