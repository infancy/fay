#include <assimp/Importer.hpp>

#include "fay/core/string.h"
#include "fay/resource/model.h"
#include "fay/resource/file.h"

// using namespace std;

namespace fay
{

assimp_model::assimp_model(const std::string& filepath, render_backend_type api, model_format model_format) : 
	resource_model(filepath, api), modeltype{ model_format }
{
	Assimp::Importer importer;
	const aiScene* scene{};
	if(api == render_backend_type::opengl)
		scene = importer.ReadFile(filepath,
			aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	else
		scene = importer.ReadFile(filepath,
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		LOG(ERROR) << "ASSIMP:: " << importer.GetErrorString();
		return;
	}

	process_node(scene->mRootNode, scene);	// process ASSIMP's root node recursively
}

void assimp_model::process_node(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene);
}

assimp_mesh assimp_model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<vertex5>  vertices;
	std::vector<uint32_t> indices;
	std::vector<std::pair<image_ptr, texture_format>> images;

	// vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		vertex5 vertex;

		auto copy = [](glm::vec3& vec, aiVector3D& rhs) { vec.x = rhs.x; vec.y = rhs.y; vec.z = rhs.z; };

		copy(vertex.position, mesh->mVertices[i]);
		copy(vertex.normal, mesh->mNormals[i]);
		if(modeltype == model_format::obj)
		{ 
			copy(vertex.tangent, mesh->mTangents[i]);
			copy(vertex.bitangent, mesh->mBitangents[i]);
		}

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.texcoord.x = mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texcoord = glm::vec2(0.f, 0.f);

		vertices.push_back(vertex);
	}

	// indices
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (uint32_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	auto load_maps = [this, &images, material](aiTextureType type, texture_format textype)
	{
		auto maps = load_images(material, type, textype);
		images.insert(images.end(), maps.begin(), maps.end());
	};

	load_maps(aiTextureType_AMBIENT,  texture_format::ambient);
	load_maps(aiTextureType_DIFFUSE,  texture_format::diffuse);
	load_maps(aiTextureType_SPECULAR, texture_format::specular);
	if (modeltype == model_format::obj)
		load_maps(aiTextureType_HEIGHT,   texture_format::parallax);
	else
		load_maps(aiTextureType_NORMALS,  texture_format::parallax);
	//std::cout << "\nnormals:";
	//load_maps(aiTextureType_NORMALS,  texture_format::normals);
	//load_maps(aiTextureType_UNKNOWN, texture_format::unknown);

	return assimp_mesh(vertices, indices, images);
}

std::vector<std::pair<image_ptr, texture_format>> 
assimp_model::load_images(aiMaterial* mat, aiTextureType type, texture_format textype)
{
	std::vector<std::pair<image_ptr, texture_format>> images;

	for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString ainame;
		mat->GetTexture(type, i, &ainame);
		std::string name{ ainame.C_Str() };

		if(!name.empty())
		{ 
			if (images_cache.find(name) == images_cache.end())
			{
				std::cout << "\nload image: " << (path + name);
				image_ptr img(path + name, true);
				images.push_back({img, textype});
				images_cache.insert({ name, img });
			}
			else
				images.push_back({images_cache[name], textype});
		}
	}
	return std::move(images);
}

} // namespace fay