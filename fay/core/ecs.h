#pragma once

#include <unordered_set>

#include "fay/core/fay.h"
#include "fay/core/hash.h"
#include "fay/core/utility.h"

namespace fay
{

template <typename T, typename Tuple>
inline T* get_component(Tuple &t)
{
    return std::get<T*>(t);
}



// template<typename Derive>
class component
{
public:
    // static constexpr inline std::type_index type_id{ typeid(Derive) };
};

using component_type_id = uint64_t;
using component_group_id = uint64_t; // component_type_group_id

#define FAY_COMPONENT_TYPE_HASH(type) \
static constexpr inline component_type_id type_id_{ fnv<component_type_id>::hash(#type) }; \
static constexpr component_type_id type_id() { return type_id_; }

struct component_mapping
{
    template <typename T>
    static constexpr component_type_id type_id()
    {
        return T::type_id();
    }

    template <typename... Ts>
    static constexpr component_group_id group_id()
    {
        return fnv<uint64_t>::merge(Ts::type_id()...);
    }
};

/*
template<typename T>
class component_set
{
public:
    void insert(T hash) // group_id
    {
        set_.insert(hash);
    }

    auto begin()
    {
        return set_.begin();
    }

    auto end()
    {
        return set_.end();
    }

private:
    std::unordered_set<T> set_;
};
*/

class entity_pool;

class entity // : std::enable_shared_from_this<entity>
{
public:
    entity() = default;

    entity(entity_pool* pool, uint64_t hash)
        : pool_(pool), hash_(hash)
    {
    }

    bool has_component(component_type_id type_id) const
    {
        auto itr = components_.find(type_id);
        return itr != components_.end();
    }

    template <typename Component>
    bool has_component() const
    {
        return has_component(component_mapping::type_id<Component>());
    }

    template <typename T>
    T* get_component()
    {
        auto t = components_.find(component_mapping::type_id<T>());
        if (t != components_.end())
            return static_cast<T*>(t->second);
        else
            return nullptr;
    }

    template <typename T>
    const T* get_component() const
    {
        auto t = components_.find(component_mapping::type_id<T>());
        if (t != components_.end())
            return static_cast<const T*>(t->second);
        else
            return nullptr;
    }

    template <typename T, typename... Ts>
    T* create_component(Ts&&... ts);

    template <typename T>
    void destroy_component();

    auto& components()
    {
        return components_;
    }

    entity_pool* pool()
    {
        return pool_;
    }

    component_type_id hash() const
    {
        return hash_;
    }

private:
    friend class entity_pool;

    uint64_t hash_;
    uint64_t pool_index_{}; // used in destroying this entity in entity_pool

    entity_pool* pool_{};
    std::unordered_map<component_type_id, component*> components_;
};

using entity_ptr = std::unique_ptr<entity>;

class base_entity_group
{
public:
    virtual ~base_entity_group() = default;
    virtual void add_entity(entity& entity) = 0;
    virtual void remove_entity(const entity& entity) = 0;
};

template <typename... Ts>
class entity_group : public base_entity_group
{
public:
    // if entity have all of Ts..., add it to group
    void add_entity(entity& entity) final
    {
        if (has_all_components<Ts...>(entity))
        {
            entity_to_index_[entity.hash()] = entities_.size();
            groups_.push_back(std::make_tuple(entity.get_component<Ts>()...));
            entities_.push_back(&entity);
        }
    }

    void remove_entity(const entity& entity) final
    {
        if (entity_to_index_.find(entity.hash()) != entity_to_index_.end())
        {
            size_t index = entity_to_index_[entity.hash()];

            entities_[index] = entities_.back(); // unsort_remove
            groups_[index] = groups_.back();
            entity_to_index_[entities_[index]->hash()] = index; // need to use entities here

            entity_to_index_.erase(entity.hash());
            entities_.pop_back();
            groups_.pop_back();
        }
    }

    std::vector<std::tuple<Ts*...>> &groups()
    {
        return groups_;
    }

private:
    template <typename... Us>
    struct HasAllComponents;

    template <typename U, typename... Us>
    struct HasAllComponents<U, Us...>
    {
        static bool has_component(const entity& entity)
        {
            return entity.has_component(component_mapping::type_id<U>()) &&
                HasAllComponents<Us...>::has_component(entity);
        }
    };

    template <typename U>
    struct HasAllComponents<U>
    {
        static bool has_component(const entity& entity)
        {
            return entity.has_component(component_mapping::type_id<U>());
        }
    };

    template <typename... Us>
    bool has_all_components(const entity& entity)
    {
        return HasAllComponents<Us...>::has_component(entity);
    }

private:
    std::vector<std::tuple<Ts*...>> groups_{};
    std::vector<const entity*> entities_{};
    std::unordered_map<uint64_t, size_t> entity_to_index_{}; // hash_to_index
};

class entity_pool
{
public:
    entity_pool() = default;
    entity_pool(const entity_pool& that) = delete;
    entity_pool& operator=(const entity_pool& that) = delete;
    entity_pool(entity_pool&& temp) = default;
    entity_pool& operator=(entity_pool&& temp) = default;
    ~entity_pool();

    entity* create_entity();
    void destroy_entity(entity *entity);

    template <typename T, typename... Ts>
    T* create_component_(entity& entity, Ts&&... ts)
    {
        auto type_id = component_mapping::type_id<T>();
        // t = new ComponentAllocator<T>();
        // auto* allocator = static_cast<ComponentAllocator<T> *>(t);

        auto iter = entity.components_.find(type_id);
        if (iter != entity.components_.end())
        {
            /*
            auto* comp = static_cast<T*>(iter->second);
            // In-place modify. Destroy old data, and in-place construct.
            comp->~T();
            return new(comp) T{ std::forward<Ts>(ts)... };
            */
            LOG(ERROR) << "shouldn't create component that already exist";
            return nullptr;
        }
        else
        {
            auto* comp = new T{ std::forward<Ts>(ts)... };
            components_.emplace_back(comp);
            entity.components_.insert({ type_id, comp });

            auto iter = component_group_sets_.find(type_id);
            if (iter != component_group_sets_.end())
                for (auto group_id : iter->second) // for(auto id : component_set)
                    entity_groups_[group_id]->add_entity(entity);

            return comp;
        }
    }
    void destroy_component_(const entity& entity, component_type_id id, component* component);

    template <typename... Ts>
    std::vector<std::tuple<Ts*...>>& get_component_group()
    {
        auto group_id = component_mapping::group_id<Ts...>();

        auto iter = entity_groups_.find(group_id);
        if (iter == entity_groups_.end())
        {
            register_group<Ts...>(group_id); // component_group_sets_

            base_entity_group* ptr = new entity_group<Ts...>();
            entity_groups_[group_id].reset(ptr);

            auto* group = static_cast<entity_group<Ts...>*>(ptr);
            for (auto& entity : entities_)
                group->add_entity(*entity);
        }

        auto* group = static_cast<entity_group<Ts...>*>(entity_groups_[group_id].get());
        return group->groups();
    }

    void clear_groups();

private:
    using component_group_sets = std::unordered_map<component_type_id, std::unordered_set<component_group_id>>;
    // if constexpr(sizeof...(args) == 1)

    template <typename... Us>
    struct GroupRegisters;

    template <typename U, typename... Us>
    struct GroupRegisters<U, Us...>
    {
        static void register_group(component_group_sets& sets, component_group_id group_id)
        {
            sets[component_mapping::type_id<U>()].insert(group_id);
            GroupRegisters<Us...>::register_group(sets, group_id);
        }
    };

    template <typename U>
    struct GroupRegisters<U>
    {
        static void register_group(component_group_sets& sets, component_group_id group_id)
        {
            sets[component_mapping::type_id<U>()].insert(group_id);
        }
    };

    // auto group_id = component_mapping::group_id<Ts...>();
    template <typename U, typename... Us>
    void register_group(component_group_id group_id)
    {
        GroupRegisters<U, Us...>::register_group(component_group_sets_, group_id);
    }

private:
    // object_pool<entity> entity_pool_;
    // std::vector<entity*> entities_;

    // TODO: ptr_container
    // TODO: remove it, manage component by entity itself.
    std::vector<std::unique_ptr<component>> components_;
    std::vector<entity_ptr> entities_;
    uint64_t cookie{};

    // cache
    component_group_sets component_group_sets_; // component_set<group_id>
    std::unordered_map<component_group_id, std::unique_ptr<base_entity_group>> entity_groups_; // group_id
};

// TODO: Ts -> Args
template <typename T, typename... Ts>
T* entity::create_component(Ts&&... ts)
{
    return pool_->create_component_<T>(*this, std::forward<Ts>(ts)...);
}

template <typename T>
void entity::destroy_component()
{
    auto id = component_mapping::type_id<T>();
    auto* t = components.find(id);
    if (t)
    {
        components.erase(t);
        pool_->destroy_component_(*this, t->get_hash(), t);
    }
}

} // namespace fay
