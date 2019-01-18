#include <assimp/Importer.hpp>
//#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "fay/core/string.h"
#include "fay/resource/model.h"
#include "fay/resource/file.h"

// using namespace std;

namespace fay
{

struct assimp_vertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texcoord{}; 
    glm::vec3 tangent{};
    glm::vec3 bitangent{};

    assimp_vertex() = default;

    assimp_vertex(glm::vec3 p, glm::vec3 n, glm::vec2 t, glm::vec3 tan, glm::vec3 bit) 
        : position{ p }
        , normal{ n }
        , texcoord{ t }
        , tangent{ tan }
        , bitangent{ bit }
    {
    }
};

/*
class model_assimp : public resource_model
{
public:
    model_assimp(const std::string& filepath, render_backend_type api = render_backend_type::opengl, model_format model_format = model_format::obj);

private:
    void convert_node(aiNode* node, const aiScene* scene);
    assimp_mesh convert_mesh(aiMesh* mesh, const aiScene* scene);

    std::vector<std::pair<image, texture_format>>
        load_images(aiMaterial* mat, aiTextureType type, texture_format textype);

public:
    std::vector<assimp_mesh> meshes;

private:
    model_format modeltype;
    std::unordered_map<std::string, image> images_cache;	// 保存已加载的图像，避免重复加载
};
*/

class model_assimp : public resource_model
{
public:
    model_assimp(const std::string& filepath, render_backend_type api);

private:
    size_t convert_nodes(std::vector<resource_node>& nodes, const aiNode* aiNode);

    void convert(resource_mesh& mesh, const aiMesh* aiMesh);
    void convert(resource_material& mat, const aiMaterial* aiMat);

private:
    // std::unordered_map<std::string, image_sp> images_cache;	// TODO: filessystem, path
};

resource_model_ptr create_model_assimp(const std::string& filepath, render_backend_type api)
{
    // static model_assimp loader;

    return std::make_unique<model_assimp>(filepath, api);
}

model_assimp::model_assimp(const std::string& filepath, render_backend_type api)
    : resource_model(filepath, api)
{
    Assimp::Importer importer_{};
	const aiScene* scene{};

	if(api == render_backend_type::opengl)
		scene = importer_.ReadFile(filepath,
			aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	else
		scene = importer_.ReadFile(filepath,
			aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);

	if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		LOG(ERROR) << "ASSIMP:: " << importer_.GetErrorString();
		return;
	}

    convert_nodes(nodes_, scene->mRootNode);
    root_node_ = nodes_.front();

    auto convert_vector = [this](auto& dsts, auto** srcs, size_t size)
    {
        dsts.clear();
        dsts.reserve(size);
        dsts.resize(size);

        for (size_t i = 0; i < size; ++i)
            convert(dsts[i], srcs[i]);
    };

    convert_vector(meshes_, scene->mMeshes, scene->mNumMeshes);
    convert_vector(materials_, scene->mMaterials, scene->mNumMaterials);
}

// Pre-Order Traversal
size_t model_assimp::convert_nodes(std::vector<resource_node>& nodes, const aiNode* aiNode)
{
    size_t node_index = nodes.size();
    nodes.emplace_back(resource_node());
    auto& node = nodes[node_index];

    node.name = aiNode->mName.C_Str();
    auto mat4 = aiNode->mTransformation;// This is *always* a row-major matrix
    mat4.Transpose(); // WARNNING: fix after use fay::mat4
    std::memcpy(&node.transform, &mat4, sizeof(mat4));

    for (size_t i = 0; i < aiNode->mNumMeshes; ++i)
        node.meshes.push_back(aiNode->mMeshes[i]);

    for (size_t i = 0; i < aiNode->mNumChildren; ++i)
    {
        size_t index = convert_nodes(nodes, aiNode->mChildren[i]);
        nodes[node_index].children.push_back(index); // WARNNING: avoid iterator invalidation
    }

    return node_index;
}

void model_assimp::convert(resource_mesh& mesh, const aiMesh* aiMesh)
{
	std::vector<assimp_vertex> vertices;
	std::vector<uint32_t> indices;

	// vertices
	for (uint32_t i = 0; i < aiMesh->mNumVertices; ++i)
	{
        assimp_vertex vertex;

		auto copy = [](glm::vec3& vec, aiVector3D& rhs) { vec.x = rhs.x; vec.y = rhs.y; vec.z = rhs.z; };

		copy(vertex.position, aiMesh->mVertices[i]);
		copy(vertex.normal, aiMesh->mNormals[i]);

		if (aiMesh->mTextureCoords[0]) // does the aiMesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.texcoord.x = aiMesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = aiMesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texcoord = glm::vec2(0.f, 0.f);

        if (format() == model_format::obj)
        {
            copy(vertex.tangent, aiMesh->mTangents[i]);
            copy(vertex.bitangent, aiMesh->mBitangents[i]);
        }

		vertices.push_back(vertex);
	}

	// indices
	for (uint32_t i = 0; i < aiMesh->mNumFaces; ++i)
	{
		aiFace face = aiMesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (uint32_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// convert
    mesh.name = aiMesh->mName.C_Str();

    mesh.layout = vertex_layout{
        {fay::attribute_usage::position,  fay::attribute_format::float3},
        {fay::attribute_usage::normal,    fay::attribute_format::float3},
        {fay::attribute_usage::texcoord0, fay::attribute_format::float2},
        {fay::attribute_usage::tangent,   fay::attribute_format::float3},
        {fay::attribute_usage::bitangent, fay::attribute_format::float3},
    };
    DCHECK(mesh.layout.stride() == 56);

    mesh.size = aiMesh->mNumVertices;
    size_t byte_size = mesh.size * mesh.layout.stride(); // magic number
    mesh.vertices.reserve(byte_size);
    mesh.vertices.resize(byte_size);
    std::memcpy(mesh.vertices.data(), vertices.data(), byte_size);

    mesh.indices = std::move(indices);

    mesh.material_index = aiMesh->mMaterialIndex;
}

void model_assimp::convert(resource_material& mtl, const aiMaterial* aiMtl)
{
    aiString ai_name;
    aiColor4D ai_albedo(0.f, 0.f, 0.f, 0.f);
    float ai_opacity = 1.f;
    aiColor4D ai_metallic(0.f, 0.f, 0.f, 0.f);
    aiColor4D ai_roughness(0.f, 0.f, 0.f, 0.f);
    //float ai_shininess = 1.f;
    aiColor4D ai_emissive(0.f, 0.f, 0.f, 0.f);
    int ai_two_sided = 0;
    float ai_alpha_test = 0;

    if (AI_SUCCESS == aiGetMaterialString(aiMtl, AI_MATKEY_NAME, &ai_name))
    {
        mtl.name = ai_name.C_Str();
    }

    /*
    if (format() == model_format::gltf)
    {
        if (AI_SUCCESS == aiGetMaterialColor(aiMtl, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_FACTOR, &ai_albedo))
        {
            albedo = Color4ToFloat3(ai_albedo);
            opacity = ai_albedo.a;
        }
    }
    else
    */
    {
        if (AI_SUCCESS == aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_DIFFUSE, &ai_albedo))
        {
            mtl.albedo_factor = glm::vec4(ai_albedo.r, ai_albedo.g, ai_albedo.b, ai_albedo.a);
        }
        if (AI_SUCCESS == aiGetMaterialFloat(aiMtl, AI_MATKEY_OPACITY, &ai_opacity))
        {
            mtl.albedo_factor.a = ai_opacity;
        }
    }

    /*
    if (format() == model_format::gltf)
    {
        if (AI_SUCCESS == aiGetMaterialFloat(aiMtl, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, &ai_metallic))
        {
            metalness = ai_metallic;
        }
        if (AI_SUCCESS == aiGetMaterialFloat(aiMtl, AI_MATKEY_SHININESS, &ai_shininess))
        {
            shininess = ai_shininess;
        }
        shininess = MathLib::clamp(shininess, 1.0f, MAX_SHININESS);
    }
    else
    */
    {
        // magic trick
        // error: stack around the variable 'ai_roughness' was corrupted
        // if (AI_SUCCESS == aiGetMaterialFloat(aiMtl, AI_MATKEY_COLOR_AMBIENT, &ai_roughness))
        if (AI_SUCCESS == aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_AMBIENT, &ai_roughness))
        {
            mtl.roughness_factor = ai_roughness.r;
        }
        if (AI_SUCCESS == aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_SPECULAR, &ai_metallic))
        {
            mtl.metallic_factor = ai_metallic.r;
        }
    }

    if (AI_SUCCESS == aiGetMaterialColor(aiMtl, AI_MATKEY_COLOR_EMISSIVE, &ai_emissive))
    {
        mtl.emissive_factor = glm::vec4(ai_emissive.r, ai_emissive.g, ai_emissive.b, ai_emissive.a);
    }

    if ((mtl.albedo_factor.a < 1) || (aiGetMaterialTextureCount(aiMtl, aiTextureType_OPACITY) > 0))
    {
        mtl.transparent = true;
    }

    if (AI_SUCCESS == aiGetMaterialInteger(aiMtl, AI_MATKEY_TWOSIDED, &ai_two_sided))
    {
        mtl.two_sided = ai_two_sided ? true : false;
    }


    // load image


    auto try_load_image = [aiMtl, dir = directory(), flip = need_flip_image()](aiTextureType type, image& img)
    {
        auto count = aiGetMaterialTextureCount(aiMtl, aiTextureType_DIFFUSE);
        if (count > 0)
        {
            aiString image_location;
            aiGetMaterialTexture(aiMtl, aiTextureType_DIFFUSE, 0, &image_location, 0, 0, 0, 0, 0, 0);

            img = image(dir + image_location.C_Str(), flip);
        }
    };

    try_load_image(aiTextureType_DIFFUSE, mtl.albedo);

    /*
    if (format() == model_format::gltf)
    {
        count = aiGetMaterialTextureCount(aiMtl, aiTextureType_UNKNOWN);
        if (count > 0)
        {
            aiGetMaterialTexture(aiMtl, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &image_location, 0, 0, 0, 0, 0, 0);

            mtl.tex_names[RenderMaterial::TS_Glossiness] = image_location.C_Str();
        }
        else
        {
            count = aiGetMaterialTextureCount(aiMtl, aiTextureType_SHININESS);
            if (count > 0)
            {
                aiString image_location;
                aiGetMaterialTexture(aiMtl, aiTextureType_SHININESS, 0, &image_location, 0, 0, 0, 0, 0, 0);

                mtl.tex_names[RenderMaterial::TS_Glossiness] = image_location.C_Str();
            }
        }
    }
    else
    */
    {
        aiString ambi_loc; 
        aiString spec_loc;
        auto ambi_num = aiGetMaterialTextureCount(aiMtl, aiTextureType_AMBIENT);
        auto spec_num = aiGetMaterialTextureCount(aiMtl, aiTextureType_SPECULAR);
        if (ambi_num > 0)
        {
            aiGetMaterialTexture(aiMtl, aiTextureType_DIFFUSE, 0, &ambi_loc, 0, 0, 0, 0, 0, 0);
        }
        if (spec_num > 0)
        {
            aiGetMaterialTexture(aiMtl, aiTextureType_DIFFUSE, 0, &spec_loc, 0, 0, 0, 0, 0, 0);
        }
        mtl.metallic_roughness = convert_to_metallic_roughness(directory(), std::string(ambi_loc.C_Str()), std::string(spec_loc.C_Str()), need_flip_image());
    }
    
    if (format() == model_format::obj)
        try_load_image(aiTextureType_HEIGHT, mtl.normal);
    else
        try_load_image(aiTextureType_NORMALS, mtl.normal);

    try_load_image(aiTextureType_EMISSIVE, mtl.emissive);
}

} // namespace fay