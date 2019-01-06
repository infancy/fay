#pragma once

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "fay/math/bounds.h"
#include "fay/resource/define.h"
#include "fay/resource/file.h"
#include "fay/resource/image.h"

namespace fay
{

// TODO: #include "fay/math/geometry.h"
// TODO: add : model info, save (sub)model
// model_loader, scene_loader

class resource_model
{
public:
	resource_model(const std::string& filepath, render_backend_type api) :
        directory_{ get_directory(filepath) }, api{ api } {}

    std::string directory() const { return directory_; }

private:
    const std::string directory_;
    // resources_directory
    render_backend_type api;

    // glm::vec3 min{}, max{};

};

class obj_model : public resource_model
{
public:
	obj_model(const std::string& filepath, render_backend_type api = render_backend_type::opengl);

    bounds3 bounds() { return bounds3(); }

    std::vector<resource_mesh> meshes() { return meshes_; }
    std::vector<resource_material> materials() { return materials_; }

private:
	std::vector<resource_mesh> meshes_;
    std::vector<resource_material> materials_;
    std::vector<resource_node> nodes_;
};

// std::vector<mesh> create_meshes(const obj_model& obj);

/*
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
*/

} // namespace fay