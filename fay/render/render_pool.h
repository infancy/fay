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
uint type##_unique_counter{};              \
std::unordered_map<uint, type> type##_map; \
                                               \
type##_id insert(const type##_desc& desc)      \
{                                              \
    uint id = ++type##_unique_counter;     \
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
        uint id = ++unique_counter;

        // TODO: pair, make_pair, undered_map.emplace
        // std::pair<uint, Desc> p2(id, desc);
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
    uint unique_counter{};
    std::unordered_map<uint, Value> map; // xx_id dosn't meet the hash requirements
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
    buffer_id   insert(const   buffer_desc& desc) { uint pid = ++cnt[0];   buffer_map.emplace(pid, desc); return   buffer_id(pid); }
    texture_id  insert(const  texture_desc& desc) { uint pid = ++cnt[1];  texture_map.emplace(pid, desc); return  texture_id(pid); }
    shader_id   insert(const   shader_desc& desc) { uint pid = ++cnt[2];   shader_map.emplace(pid, desc); return   shader_id(pid); }
    pipeline_id insert(const pipeline_desc& desc) { uint pid = ++cnt[3]; pipeline_map.emplace(pid, desc); return pipeline_id(pid); }
    frame_id    insert(const    frame_desc& desc) { uint pid = ++cnt[4];    frame_map.emplace(pid, desc); return    frame_id(pid); }

    buffer_id   insert(uint pid, const   buffer_desc& desc) {   buffer_map.emplace(pid, desc); return   buffer_id(pid); }
    texture_id  insert(uint pid, const  texture_desc& desc) {  texture_map.emplace(pid, desc); return  texture_id(pid); }
    shader_id   insert(uint pid, const   shader_desc& desc) {   shader_map.emplace(pid, desc); return   shader_id(pid); }
    pipeline_id insert(uint pid, const pipeline_desc& desc) { pipeline_map.emplace(pid, desc); return pipeline_id(pid); }
    frame_id    insert(uint pid, const    frame_desc& desc) {    frame_map.emplace(pid, desc); return    frame_id(pid); }

    template<typename Handle>
    bool have(Handle id) const { const auto& map = get_map(id); return map.find(id.value) != map.end(); }

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
    constexpr auto& get_map(  buffer_id id) { return buffer_map;   }
    constexpr auto& get_map( texture_id id) { return texture_map;  }
    constexpr auto& get_map(  shader_id id) { return shader_map;   }
    constexpr auto& get_map(pipeline_id id) { return pipeline_map; }
    constexpr auto& get_map(   frame_id id) { return frame_map;    }

    constexpr auto& get_map(  buffer_id id) const { return buffer_map;   }
    constexpr auto& get_map( texture_id id) const { return texture_map;  }
    constexpr auto& get_map(  shader_id id) const { return shader_map;   }
    constexpr auto& get_map(pipeline_id id) const { return pipeline_map; }
    constexpr auto& get_map(   frame_id id) const { return frame_map;    }

private:
    std::array<uint, 5> cnt{ 0, 0, 0, 0, 0 };

    std::unordered_map<uint, Buffer>   buffer_map;
    std::unordered_map<uint, Texture>  texture_map;
    std::unordered_map<uint, Shader>   shader_map;
    std::unordered_map<uint, Pipeline> pipeline_map;
    std::unordered_map<uint, Frame>    frame_map;
};



// common interface for render_device
class render_desc_pool
{
public:
    // get_desc
    virtual const   buffer_desc& get(  buffer_id id) = 0;
    virtual const  texture_desc& get( texture_id id) = 0;
    virtual const   shader_desc& get(  shader_id id) = 0;
    virtual const pipeline_desc& get(pipeline_id id) = 0;
    virtual const    frame_desc& get(   frame_id id) = 0;
    
    // virtual frame_desc& get(frame_id id, uint elem_index) = 0;
};

template<typename Buffer, typename Texture, typename Shader, typename Pipeline, typename Frame>
class resource_pool_ : public render_desc_pool
{
public:
    template<typename Descriptor, typename Value>
    struct desc_value_pair_
    {
        const Descriptor desc;
        Value value;

        desc_value_pair_(Descriptor desc) : desc{ desc }, value{ desc } {}
    };

    using   buffer_pair = desc_value_pair_<  buffer_desc, Buffer>;
    using  texture_pair = desc_value_pair_< texture_desc, Texture>;
    using   shader_pair = desc_value_pair_<  shader_desc, Shader>;
    using pipeline_pair = desc_value_pair_<pipeline_desc, Pipeline>;
    using    frame_pair = desc_value_pair_<   frame_desc, Frame>;

public:
    buffer_id   insert(const   buffer_desc& desc) { uint pid = ++cnt[0];   buffer_map.emplace(pid, desc); return   buffer_id(pid); }
    texture_id  insert(const  texture_desc& desc) { uint pid = ++cnt[1];  texture_map.emplace(pid, desc); return  texture_id(pid); }
    shader_id   insert(const   shader_desc& desc) { uint pid = ++cnt[2];   shader_map.emplace(pid, desc); return   shader_id(pid); }
    pipeline_id insert(const pipeline_desc& desc) { uint pid = ++cnt[3]; pipeline_map.emplace(pid, desc); return pipeline_id(pid); }
    frame_id    insert(const    frame_desc& desc) { uint pid = ++cnt[4];    frame_map.emplace(pid, desc); return    frame_id(pid); }

    /*
    buffer_id   insert(uint pid, const   buffer_desc& desc) {   buffer_map.emplace(pid, desc); return   buffer_id(pid); }
    texture_id  insert(uint pid, const  texture_desc& desc) {  texture_map.emplace(pid, desc); return  texture_id(pid); }
    shader_id   insert(uint pid, const   shader_desc& desc) {   shader_map.emplace(pid, desc); return   shader_id(pid); }
    pipeline_id insert(uint pid, const pipeline_desc& desc) { pipeline_map.emplace(pid, desc); return pipeline_id(pid); }
    frame_id    insert(uint pid, const    frame_desc& desc) {    frame_map.emplace(pid, desc); return    frame_id(pid); }
    */

    template<typename Handle>
    bool contain(Handle id) const { const auto& map = get_map(id); return map.find(id.value) != map.end(); }

    // interface provided for render_device
    virtual const   buffer_desc& get(  buffer_id id) override { return   buffer_map[id.value].desc; }
    virtual const  texture_desc& get( texture_id id) override { return  texture_map[id.value].desc; }
    virtual const   shader_desc& get(  shader_id id) override { return   shader_map[id.value].desc; }
    virtual const pipeline_desc& get(pipeline_id id) override { return pipeline_map[id.value].desc; }
    virtual const    frame_desc& get(   frame_id id) override { return    frame_map[id.value].desc; }

    // TODO: add const
    buffer_pair&   operator[](  buffer_id id) { return   buffer_map[id.value]; }
    texture_pair&  operator[]( texture_id id) { return  texture_map[id.value]; }
    shader_pair&   operator[](  shader_id id) { return   shader_map[id.value]; }
    pipeline_pair& operator[](pipeline_id id) { return pipeline_map[id.value]; }
    frame_pair&    operator[](   frame_id id) { return    frame_map[id.value]; }

    // TODO: cache them
    // Or don't make this improvement, after all, maybe there's little performance boost.
    void erase(  buffer_id id) {   buffer_map.erase(id.value); }
    void erase( texture_id id) {  texture_map.erase(id.value); }
    void erase(  shader_id id) {   shader_map.erase(id.value); }
    void erase(pipeline_id id) { pipeline_map.erase(id.value); }
    void erase(   frame_id id) {    frame_map.erase(id.value); }

private:
    constexpr auto& get_map(  buffer_id id) { return buffer_map;   }
    constexpr auto& get_map( texture_id id) { return texture_map;  }
    constexpr auto& get_map(  shader_id id) { return shader_map;   }
    constexpr auto& get_map(pipeline_id id) { return pipeline_map; }
    constexpr auto& get_map(   frame_id id) { return frame_map;    }

    constexpr auto& get_map(  buffer_id id) const { return buffer_map;   }
    constexpr auto& get_map( texture_id id) const { return texture_map;  }
    constexpr auto& get_map(  shader_id id) const { return shader_map;   }
    constexpr auto& get_map(pipeline_id id) const { return pipeline_map; }
    constexpr auto& get_map(   frame_id id) const { return frame_map;    }

private:
    std::array<uint, 5> cnt{ 0, 0, 0, 0, 0 };

    std::unordered_map<uint,   buffer_pair>   buffer_map;
    std::unordered_map<uint,  texture_pair>  texture_map;
    std::unordered_map<uint,   shader_pair>   shader_map;
    std::unordered_map<uint, pipeline_pair> pipeline_map;
    std::unordered_map<uint,    frame_pair>    frame_map;
};

/*

inline namespace type
{

struct buffer
{
    buffer(buffer_desc desc)
    {

    }
};

struct texture
{
    texture(texture_desc desc)
    {

    }
};

struct shader
{
    shader(shader_desc desc)
    {

    }
};

struct pipeline
{
    pipeline(pipeline_desc desc)
    {

    }
};

struct frame
{
    frame(frame_desc desc)
    {

    }
};

}

*/

} // namespace fay
