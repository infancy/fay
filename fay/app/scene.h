#pragma once

#include "fay/app/components.h"
#include "fay/app/app_flexible.h"
#include "fay/core/ecs.h"
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

// scene_node, base_node
class node;
FAY_SHARED_PTR(node)

class node : std::enable_shared_from_this<node>
{
public:
    node() = default;

    node(std::string name, entity* e)
        : name_{ name }
        , entity_{ e }
    {
    }

    void add_children(node_sp node) // const node_sp& node, const_node_sp& node
    {
        node->parent_ = weak_from_this();
        children_.push_back(node);
    }

    void remove_children(node_sp node)
    {
        // TODO
    }

private:
    std::string name_;
    entity* entity_{};

    node_wp parent_{};
    std::vector<node_sp> children_;

};


class graphics_scene
{
public:

    std::vector<std::tuple<transform_component*, renderable_component*>>* renderables;

};

// create, manage, destroy
class scene
{
public:
    scene(render_device* device)
    {
        device_ = device;

        root_ = std::make_shared<node>();
    }
    // scene load_scene(render_device_ptr& device, const std::string& filename)

    node_wp add_model(const std::string& model_path, node* node = nullptr)
    {
        auto model = create_resource_model(model_path, device_->type());
        auto meshes = create_renderables(*model, device_);

        auto model_root = build_tree(model->root_node(), model->nodes(), meshes);

        if (node)
            node->add_children(model_root);
        else
            root_->add_children(model_root);

        return model_root;
    }



    graphics_scene graphics_scene_proxy()
    {
        graphics_scene gfx;
        gfx.renderables = &(pool_.get_component_group<transform_component, renderable_component>());

        return gfx;
    }

private:
    node_sp build_tree(resource_node _resource_node, const std::vector<resource_node> nodes, const std::vector<renderable_sp>& meshes)
    {
        auto* entity = pool_.create_entity();
        auto* comp = entity->create_component<transform_component>();
        comp->transform = _resource_node.transform;

        if (auto children = _resource_node.meshes.size(); children > 0)
        {
            auto* comp = entity->create_component<renderable_component>();

            if (children == 1)
            {
                comp->renderable = meshes[_resource_node.meshes.front()];
            }
            else // many mesh
            {
                comp->renderable = std::make_shared<array_mesh>(meshes, _resource_node.meshes);
            }
        }

        // TODO: resoucre management of node and entity
        // constant, update, query
        auto subtree_root = std::make_shared<node>(_resource_node.name, entity);

        for (auto index : _resource_node.children)
        {
            subtree_root->add_children(build_tree(nodes[index], nodes, meshes));
        }

        return subtree_root;
    }

private:
    render_device* device_;
    entity_pool pool_;

    node_sp root_;
};

FAY_UNIQUE_PTR(scene)

// scene_manager

} // namespace fay
