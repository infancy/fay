#pragma once

#include "fay/render/define.h"

namespace fay
{

/*

// -------------------------------------------------------------------------------------------------
1. class pool : pool_<buffer>, pool_<texture>...

// -------------------------------------------------------------------------------------------------
2. class pool { FAY_RENDER_POOL( type )... };
#define FAY_RENDER_POOL( type )                \
uint32_t type##_unique_counter{};              \
std::unordered_map<uint32_t, type> type##_map; \
                                               \
type##_id insert(const type##_desc& desc)      \
{                                              \
    uint32_t id = ++type##_unique_counter;     \
    type##_map.emplace(id, desc);              \
    return type##_id(id);                      \
}                                              \
                                               \
type& operator[](type##_id key)                \
{                                              \
    return type##_map[key.id];                 \
}
#undef FAY_RENDER_POOL

// -------------------------------------------------------------------------------------------------
3.
class empty_class
{
};

template<typename Key, typename Desc, typename Value, typename Base = empty_class>
class resource_pool_ // pool_
{
public:
    Key insert(const Desc& desc) // std::pair<Key, Value&> insert(const Desc& desc) isn't a good interfae
    {
        uint32_t id = ++unique_counter;

        // TODO: pair, make_pair, undered_map.emplace
        // std::pair<uint32_t, Desc> p2(id, desc);
        // auto p = std::make_pair<Key, Value>(id, desc);
        // std::pair<Key, Value> p = std::make_pair<Key, Value>(id, desc);

        map.emplace(id, desc);

        return Key(id);
    }

    Value& operator[](Key key)
    {
        return map[key.id];
    }

private:
    uint32_t unique_counter{};
    std::unordered_map<uint32_t, Value> map; // xx_id dosn't meet the hash requirements
};

template<> using  buffer_pool = resource_pool_< buffer_id,  buffer_desc,  buffer, Base>;
template<> using texture_pool = resource_pool_<texture_id, texture_desc, texture, Base>;
template<> using  shader_pool = resource_pool_< shader_id,  shader_desc,  shader, Base>;

// pool<buffer, texture, shader...> pool_;
class pool :
    public buffer_pool<texture_pool<shader_pool<...>>>
{
};

*/

template<typename Buffer, typename Texture, typename Shader, typename Pipeline, typename Frame>
class resource_pool // pool
{
public:
    buffer_id   insert(const   buffer_desc& desc) { uint32_t pid = ++cnt[0];   buffer_map.emplace(pid, desc); return   buffer_id(pid); }
    texture_id  insert(const  texture_desc& desc) { uint32_t pid = ++cnt[1];  texture_map.emplace(pid, desc); return  texture_id(pid); }
    shader_id   insert(const   shader_desc& desc) { uint32_t pid = ++cnt[2];   shader_map.emplace(pid, desc); return   shader_id(pid); }
    pipeline_id insert(const pipeline_desc& desc) { uint32_t pid = ++cnt[3]; pipeline_map.emplace(pid, desc); return pipeline_id(pid); }
    frame_id    insert(const    frame_desc& desc) { uint32_t pid = ++cnt[4];    frame_map.emplace(pid, desc); return    frame_id(pid); }

    template<typename Handle>
    bool have(Handle id) const { const auto& m = map(id); return m.find(id.value) != m.end(); }

    // TODO: add const
    Buffer&   operator[](  buffer_id id) { return   buffer_map[id.value]; }
    Texture&  operator[]( texture_id id) { return  texture_map[id.value]; }
    Shader&   operator[](  shader_id id) { return   shader_map[id.value]; }
    Pipeline& operator[](pipeline_id id) { return pipeline_map[id.value]; }
    Frame&    operator[](   frame_id id) { return    frame_map[id.value]; }

    // TODO: cache them
    // Or don't make this improvement, after all, maybe there's little performance boost.
    void erase(  buffer_id id) {   buffer_map.erase(id.value); }
    void erase( texture_id id) {  texture_map.erase(id.value); }
    void erase(  shader_id id) {   shader_map.erase(id.value); }
    void erase(pipeline_id id) { pipeline_map.erase(id.value); }
    void erase(   frame_id id) {    frame_map.erase(id.value); }

private:
    constexpr auto& map(  buffer_id id) { return buffer_map;   }
    constexpr auto& map( texture_id id) { return texture_map;  }
    constexpr auto& map(  shader_id id) { return shader_map;   }
    constexpr auto& map(pipeline_id id) { return pipeline_map; }
    constexpr auto& map(   frame_id id) { return frame_map;    }

    constexpr auto& map(  buffer_id id) const { return buffer_map;   }
    constexpr auto& map( texture_id id) const { return texture_map;  }
    constexpr auto& map(  shader_id id) const { return shader_map;   }
    constexpr auto& map(pipeline_id id) const { return pipeline_map; }
    constexpr auto& map(   frame_id id) const { return frame_map;    }

private:
    std::array<uint32_t, 5> cnt{ 0, 0, 0, 0, 0 };

    std::unordered_map<uint32_t, Buffer>   buffer_map;
    std::unordered_map<uint32_t, Texture>  texture_map;
    std::unordered_map<uint32_t, Shader>   shader_map;
    std::unordered_map<uint32_t, Pipeline> pipeline_map;
    std::unordered_map<uint32_t, Frame>    frame_map;
};

} // namespace fay
