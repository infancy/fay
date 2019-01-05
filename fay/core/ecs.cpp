#include "fay/core/ecs.h"

namespace fay
{

entity_pool::~entity_pool()
{
    // TODO:
    // delete all component allocator
}

entity* entity_pool::create_entity()
{
    auto ptr = std::make_unique<entity>(this, ++cookie);
    ptr->pool_index_ = entities_.size();
    entities_.push_back(std::move(ptr));
    return entities_.back().get();
}

void entity_pool::destroy_entity(entity* entity)
{
    auto& components = entity->components();
    for (auto [id, component] : components)
    {
        destroy_component_(*entity, id, component);
    }

    auto index = entity->pool_index_;

    DCHECK(index < entities_.size());

    std::swap(entities_[index], entities_.back());
    entities_[index]->pool_index_ = index;
    entities_.pop_back();
    //entity_pool.free(entity);
}

void entity_pool::destroy_component_(const entity& entity, component_type_id id, component* component)
{
    auto idx = index(components_, [component](const auto& uptr)
    {
        return uptr.get() == component;
    });
    DCHECK(idx.has_value());

    components_[idx.value()].reset(nullptr);

    auto iter = component_group_sets_.find(id);
    if (iter != component_group_sets_.end())
    {
        for (auto group_id : iter->second)
        {
            auto g = entity_groups_.find(group_id);
            if (g != entity_groups_.end())
                g->second->remove_entity(entity);
        }
    }
}

void entity_pool::clear_groups()
{
    component_group_sets_.clear();
    entity_groups_.clear();
}

} // namespace fay