#pragma once

#include "fay/math/geometry.h"
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
    resource_model(const std::string& filepath, render_backend_type api);

    //! 
    virtual resource_node root_node() { return root_node_; }

    const std::vector<resource_node>& nodes() const { return nodes_; }

    // bounds3 bounds() { return bounds3(); }

    const std::vector<resource_mesh>& meshes() const { return meshes_; }
    const std::vector<resource_material>& materials() const { return materials_; }

protected:
    std::string directory() const;

    model_format format() const;

    bool need_flip_image() const;

protected:
    const std::string filepath_;
    const std::string dir_; // resources_directory
    model_format fmt_{};
    render_backend_type api_{};

    // glm::vec3 min{}, max{};
    resource_node root_node_{};
    std::vector<resource_node> nodes_;

    std::vector<resource_mesh> meshes_; // all submeshes in glTF
    std::vector<resource_material> materials_;
};

using resource_model_ptr = std::unique_ptr<resource_model>;

image convert_to_metallic_roughness(const std::string& directory, const std::string& ambient, const std::string& specular, bool flip_vertical);

// deprecated
resource_model_ptr create_model_obj(const std::string& filepath, render_backend_type api);

resource_model_ptr create_model_assimp(const std::string& filepath, render_backend_type api);

inline resource_model_ptr create_resource_model(const std::string& filepath, render_backend_type api = render_backend_type::opengl)
{
    //if (auto type = get_filetype(filepath); (type == "gltf") || (type == "glb"))
    //{
    //    return create_model_obj(filepath, api);
    //}
    return create_model_assimp(filepath, api);
}

/*

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