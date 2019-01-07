#pragma once

#include "fay/core/fay.h"
#include "fay/gfx/renderable.h"
#include "fay/gfx/material.h"
#include "fay/gfx/mesh.h"
#include "fay/resource/model.h"

namespace fay
{

/*
struct mesh
{
    transform_component transform_;
    renderable_component renderable_;
    physical_component physical_;
};

*/

/*
todo: class path, scene_format,  
*/

struct scene_node
{
    std::string name;
    std::vector<uint32_t> node_indices;
};

class scene
{
public:

    // submit, 
    void flush_to(command_list& cmd)
    {
        for (auto& mesh : render_list)
            mesh->render(cmd);
    }

    std::vector<material_sptr> material_list;
    std::vector<renderable_sptr> render_list;

private:
    scene_node root_node_;
};

// create, manage, destroy
class scene_manager
{
public:
    scene load_scene(render_device_ptr& device, const std::string& filename)
    {
        obj_model obj(filename);
        //for (auto& mat : obj.materials())
        //{
        //    if (!mat.metallic_roughness.empty())
        //        mat.metallic_roughness.save("test/" + mat.name);
        //}

        auto& mats = obj.materials();
        auto& meshes = obj.meshes();

        scene scene_;

        for (const auto& mat : mats)
        {
            scene_.material_list.emplace_back(std::make_shared<material>(device, mat));
        }

        for (const auto& mesh : meshes)
        {
            scene_.render_list.emplace_back(std::make_shared<static_mesh>(device, mesh, scene_.material_list[mesh.material_index]));
        }

        return std::move(scene_);
    }
};

} // namespace fay
