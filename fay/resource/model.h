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
        dir_{ get_directory(filepath) }, api_{ api } {}

    // std::string directory() const { return directory_; }

    //! 
    virtual resource_node root_node() { return root_node_; }

    const std::vector<resource_node>& nodes() const { return nodes_; }

    // bounds3 bounds() { return bounds3(); }

    const std::vector<resource_mesh>& meshes() const { return meshes_; }
    const std::vector<resource_material>& materials() const { return materials_; }

private:

protected:
    const std::string dir_; // resources_directory
    render_backend_type api_{};

    // glm::vec3 min{}, max{};
    resource_node root_node_{};
    std::vector<resource_node> nodes_;

    std::vector<resource_mesh> meshes_; // all submeshes in glTF
    std::vector<resource_material> materials_;
};

using resource_model_ptr = std::unique_ptr<resource_model>;

// deprecated
resource_model_ptr create_model_obj(const std::string& filepath, render_backend_type api);

inline resource_model_ptr create_resource_model(const std::string& filepath, render_backend_type api = render_backend_type::opengl)
{
    //if (auto type = get_filetype(filepath); (type == "gltf") || (type == "glb"))
    //{
    //    return create_model_obj(filepath, api);
    //}
    if (get_filetype(filepath) == "obj")
    {
        return create_model_obj(filepath, api);
    }
    else
    {
        return {};
    }
}

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

class resource_scene
{
public:
    resource_model(const std::string& filepath, render_backend_type api) :
        dir_{ get_directory(filepath) }, api_{ api } {}

    // std::string directory() const { return directory_; }

    //!
    virtual resource_scene default_scene() { return default_scene_; }

    const std::vector<resource_scene>& scenes() const { return scenes_; }
    const std::vector<resource_node>& nodes() const { return nodes_; }

    // bounds3 bounds() { return bounds3(); }

    const std::vector<resource_mesh>& meshes() const { return meshes_; }
    const std::vector<resource_material>& materials() const { return materials_; }

private:

protected:
    const std::string dir_; // resources_directory
    render_backend_type api_{};

    // glm::vec3 min{}, max{};
    resource_scene default_scene_{};
    std::vector<resource_scene> scenes_;
    std::vector<resource_node> nodes_;

    std::vector<resource_mesh> meshes_; // all submeshes in glTF
    std::vector<resource_material> materials_;
};
*/

} // namespace fay