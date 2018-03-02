#include "load_model.h"
#include "fay/utility/file.h"
#include <assimp/Importer.hpp>

// using namespace std;

namespace fay
{

BaseModel::BaseModel(const std::string& filepath, Thirdparty api) : 
	path{ get_path(filepath) }, api{ api } {}

// load obj model --------------------------------------------------------------

ObjModel::ObjModel(const std::string& filepath, Thirdparty api) : BaseModel(filepath, api)
{
}

// 将位置、索引转换成纹理数据
/*
void objMesh_transform_to_TextureDataArray(
	std::vector<ObjMesh>& meshes,
	std::vector<glm::vec4>& positions,
	std::vector<glm::uvec4>& indices,
	std::vector<std::string>& texpaths)
{
	// 考虑到合并了重复的纹理，因此 texpaths 可能更小一点
	// positions[p1, p2, ...... pn]
	// indices  [i1, i2, ...... in]
	// texpaths [t1, t2, ...... tn]

	positions.clear(); // positions.reserve(mesh.size())
	indices.clear();
	for (auto& mesh : meshes)
	{
		for (auto& vertex : mesh.vertices)
			positions.emplace_back(glm::vec4{ vertex.position, 0.f });

		uint32_t tex_index{};
		if (!mesh.images.empty())
		{
			// 只使用第一张纹理
			std::string tex_filepath = { mesh.images[0].first.filepath };

			if (std::find(texpaths.begin(), texpaths.end(), tex_filepath) == texpaths.end())
				texpaths.push_back(tex_filepath);

			for (; tex_index < texpaths.size(); ++tex_index)
				if (texpaths[tex_index] == tex_filepath)
					break;
		}
		else
			tex_index = 255;

		// 在着色器中通过 tex_index，即纹理的下标，找到对应的纹理
		auto& indices = mesh.indices;
		for (size_t j{}; j < indices.size(); j += 3)
			indices.emplace_back(glm::vec4{ indices[j], indices[j + 1], indices[j + 2], tex_index });
	}
	LOG(INFO) << " transform_to_TextureDataArray: "
		<< " positions: " << positions.size()
		<< " indices: " << indices.size()
		<< " texpaths: " << texpaths.size();
}
*/
// load model by assimp --------------------------------------------------------

AssimpModel::AssimpModel(const std::string& filepath, Thirdparty api) : BaseModel(filepath, api)
{
	Assimp::Importer importer;
	// TODO: if(opengl)
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		LOG(ERROR) << "ASSIMP:: " << importer.GetErrorString();
		return;
	}

	process_node(scene->mRootNode, scene);	// process ASSIMP's root node recursively
}

void AssimpModel::process_node(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene);
}

AssimpMesh AssimpModel::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex5>  vertices;
	std::vector<uint32_t> indices;
	std::vector<std::pair<Image, TexType>> images;

	// vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex5 vertex;

		auto copy = [](glm::vec3& vec, aiVector3D& rhs) { vec.x = rhs.x; vec.y = rhs.y; vec.z = rhs.z; };

		copy(vertex.position, mesh->mVertices[i]);
		copy(vertex.normal, mesh->mNormals[i]);
		copy(vertex.tangent, mesh->mTangents[i]);
		copy(vertex.bitangent, mesh->mBitangents[i]);

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texCoords = glm::vec2(0.f, 0.f);

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

	auto load_maps = [this, &images, material](aiTextureType type, TexType textype)
	{
		auto maps = load_images(material, type, textype);
		images.insert(images.end(), maps.begin(), maps.end());
	};

	load_maps(aiTextureType_DIFFUSE,  TexType::diffuse);
	load_maps(aiTextureType_SPECULAR, TexType::specular);
	load_maps(aiTextureType_HEIGHT,   TexType::normals);
	load_maps(aiTextureType_AMBIENT,  TexType::height);

	return AssimpMesh(vertices, indices, images);
}

std::vector<std::pair<Image, TexType>> 
AssimpModel::load_images(aiMaterial* mat, aiTextureType type, TexType textype)
{
	std::vector<std::pair<Image, TexType>> images;

	for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString ainame;
		mat->GetTexture(type, i, &ainame);
		std::string name{ ainame.C_Str() };

		if (images_cache.find(name) == images_cache.end())
		{
			Image img(path + name, api);
			images.push_back({img, textype});
			images_cache.insert({ name, img });
		}
		else
			images.push_back({images_cache[name], textype});
	}
	return std::move(images);
}

} // namespace fay