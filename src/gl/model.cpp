#include "model.h"
using namespace std;

namespace fay
{

// modelMesh Method Definitions

modelMesh::modelMesh(vector<modelVertex>& vertices, vector<unsigned int>& indices, vector<modelTexture>& textures) 
	: indices_size(indices.size()), textures(textures)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// C++ 结构体中的数据是紧密排列的
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(modelVertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(modelVertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(modelVertex), (void*)offsetof(modelVertex, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(modelVertex), (void*)offsetof(modelVertex, texCoords));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(modelVertex), (void*)offsetof(modelVertex, tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(modelVertex), (void*)offsetof(modelVertex, bitangent));

	glBindVertexArray(0);
}

void modelMesh::draw(Shader shader)
{
	// bind appropriate textures
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		// retrieve texture number (the N in diffuse_textureN)
		std::string type = textures[i].type;
		std::stringstream number;

		//switch
		if (type == "texture_diffuse")
			number << diffuseNr++; // transfer uint32_t to stream
		else if (type == "texture_specular")
			number << specularNr++;
		else if (type == "texture_normal")
			number << normalNr++;
		else if (type == "texture_height")
			number << heightNr++;

		shader.bind_texture(type + number.str(), i, textures[i].id);
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}

/*
~modelMesh::modelMesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
*/

// modelMesh Method Definitions

Model::Model(const string& path, bool gamma) : gamma_correction(gamma)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		LOG(ERROR) << "ASSIMP:: " << importer.GetErrorString();
		return;
	}
	// retrieve the resources_directory path of the filepath
	resources_directory = path.substr(0, path.find_last_of('/')) + '/';

	// process ASSIMP's root node recursively
	process_node(scene->mRootNode, scene);
}

void Model::draw(Shader shader)
{
	for (auto& mesh : meshes)
		mesh.draw(shader);
}

void Model::process_node(aiNode *node, const aiScene *scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene);
}

//static void copy(glm::vec3& vec, aiVector3D& rhs) { vec.x = rhs.x; vec.y = rhs.y; vec.z = rhs.z; }

modelMesh Model::process_mesh(aiMesh *mesh, const aiScene *scene)
{
	std::vector<modelVertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<modelTexture> textures;

	// Walk through each of the mesh's vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		modelVertex vertex;

		auto copy = [](glm::vec3& vec, aiVector3D& rhs) { vec.x = rhs.x; vec.y = rhs.y; vec.z = rhs.z; };

		copy(vertex.position,  mesh->mVertices[i]);
		copy(vertex.normal,    mesh->mNormals[i]);
		copy(vertex.tangent,   mesh->mTangents[i]);
		copy(vertex.bitangent, mesh->mBitangents[i]);

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.texCoords.x = mesh->mTextureCoords[0][i].x;
			vertex.texCoords.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	// 添加索引
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (uint32_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	auto load_maps = [this, &textures, material](aiTextureType type, std::string typeName)
	{
		std::vector<modelTexture> maps = load_textures(material, type, typeName);
		textures.insert(textures.end(), maps.begin(), maps.end());
	};

	load_maps(aiTextureType_DIFFUSE,  "texture_diffuse");
	load_maps(aiTextureType_SPECULAR, "texture_specular");
	load_maps(aiTextureType_HEIGHT,   "texture_normal");
	load_maps(aiTextureType_AMBIENT,  "texture_height");

	return modelMesh(vertices, indices, textures);
}

vector<modelTexture> Model::load_textures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<modelTexture> textures;

	for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		bool have_loaded = false;
		// 先从已加载的纹理中搜索该纹理
		for (uint32_t j = 0; j < textures_loaded.size(); ++j)
		{
			if (strcmp(textures_loaded[j].filepath.C_Str(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				have_loaded = true;
				break;
			}
		}
		if (!have_loaded)	// if texture hasn't been loaded already, load it
		{
			modelTexture texture(create_texture(str.C_Str()), typeName, str);
			textures.push_back(texture);
			textures_loaded.push_back(texture);  // modelTexture 只是一个 handle，这不会增加开销
		}
	}
	return textures;
}

GLuint Model::create_texture(const char* filename, bool gamma)
{
	string filepath = resources_directory + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		LOG(INFO) << "modelTexture successed to load at path: " << filename;
		GLenum format {};

		switch (nrComponents)
		{
		case 1: format = GL_RED; break;
		case 3: format = GL_RGB; break;
		case 4: format = GL_RGBA; break;
		default:
			LOG(ERROR) << "nrComponents failed to choose"; break;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);	//之后任何的纹理指令都作用于当前绑定的纹理
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
		LOG(ERROR) << "modelTexture failed to load at path: " << filename;
	
	stbi_image_free(data);
	return textureID;
}

}	//namespace fay
