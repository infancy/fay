//#ifdef FAY_IN_WINDOWS
#include "glad/glad.h"

#include "fay/render/define.h"
#include "fay/render/device.h"
#include "fay/render/native_type.h"

namespace fay::render // ::opengl
{

// ---------------------------------------
// helper macros

// static FAY_FORCE_INLINE void check_errors() { CHECK(glGetError() == GL_NO_ERROR); }
#define glcheck_errors() CHECK(glGetError() == GL_NO_ERROR)

// ---------------------------------------
// helper functions

// TODO: move to gl_utility.h
struct opengl_utility
{
    struct vertex_attribute_gl
    {
        GLuint        index;      // 0, 1, 2, 3
        GLint         size;       // float3 : 3, byte4: 4
        GLenum        type;       // GL_FLOAT, GL_SIZE
        GLboolean     normalized; // used for integral type, map it to -1.0~1.0(0.0~1.0)
        // GLsizei       stride;
        const GLvoid* offset;     // (void*)0, (void*)(size * sizeof(type)), ...

        // these variables are just for debug when processing a pass_desc. TODO: better way
        attribute_usage  usage;
        attribute_format format;
        uint32_t         num;

        static GLenum attribute_type(attribute_format fmt)
        {
            switch (fmt)
            {
                case fay::render::attribute_format::float1:
                case fay::render::attribute_format::float2:
                case fay::render::attribute_format::float3:
                case fay::render::attribute_format::float4:
                case fay::render::attribute_format::floatx:
                    return GL_FLOAT;
                case fay::render::attribute_format::byte4:
                case fay::render::attribute_format::byte4x:
                    return GL_BYTE;
                case fay::render::attribute_format::ubyte4:
                case fay::render::attribute_format::ubyte4x:
                    return GL_UNSIGNED_BYTE;
                case fay::render::attribute_format::short2:
                case fay::render::attribute_format::short2x:
                case fay::render::attribute_format::short4:
                case fay::render::attribute_format::short4x:
                    return GL_SHORT;
                default:
                    LOG(ERROR) << "shouldn't be here";
                    return 0;
            }
        }

        static GLboolean need_normalized(attribute_format fmt)
        {
            switch (fmt)
            {
                case fay::render::attribute_format::byte4:
                case fay::render::attribute_format::byte4x:
                case fay::render::attribute_format::ubyte4:
                case fay::render::attribute_format::ubyte4x:
                case fay::render::attribute_format::short2:
                case fay::render::attribute_format::short2x:
                case fay::render::attribute_format::short4:
                case fay::render::attribute_format::short4x:
                    return GL_TRUE;
                default:
                    return GL_FALSE;
            }
        }
    };
    
    using vertex_layout_gl = std::vector<vertex_attribute_gl>;

    struct buffer // : public buffer_desc
    {
        // init by buffer_desc
        std::string name{};
        uint32_t    size{};
        // const void* data{};
        GLenum      type{};
        GLenum      usage{};
        
        // used for vertex buffer, instance buffer
        GLsizei          stride{ 0 }; // 0 means let OpenGL computes it
        vertex_layout_gl layout{ 0 };
        // only used for instance buffer
        GLuint instance_rate{ 0 };

        // then assign others
        GLuint gid{};
        //resource_state state; // TODO: remove it???

        buffer() = default;
        buffer(const buffer_desc& desc) : layout(desc.layout.size())
        {
            name = desc.name;
            size = desc.size;
            stride = desc.stride;
            type = buffer_type_map.at(desc.type).opengl;
            usage = resource_usage_map.at(desc.usage).opengl;

            if (desc.type == buffer_type::vertex || desc.type == buffer_type::instance)
            {

                uint32_t offset = 0;
                for (uint32_t i = 0; i < desc.layout.size(); ++i)
                {
                    auto& da = desc.layout[i];
                    auto& a = layout[i];

                    a.usage  = da.usage; 
                    a.format = da.format; 
                    a.num    = da.num; // normally num is 1

                    a.index = i; // i_, ix, ic, ii
                    a.type       = vertex_attribute_gl::attribute_type(da.format);
                    a.normalized = vertex_attribute_gl::need_normalized(da.format);

                    auto[size, byte] = attribute_format_map.at(da.format);
                    a.size   = size * da.num;
                    a.offset = (GLvoid*)offset;
                    offset += byte * da.num; // e.g. {..., floatx, 8} -> size: 1 * 8, offset: 4 * 8
                }
            }

            if (desc.type == buffer_type::instance)
                instance_rate = desc.instance_rate;
        }
    };

    struct texture
    {
        // init by texture_desc
        std::string name{};

        // then assign others
        GLuint gid{}; // program_id

        texture() = default;
        texture(const texture_desc& desc)
        {
            name = desc.name;
        }
    };

    struct shader
    {
        constexpr static GLenum vs_type = GL_VERTEX_SHADER;
        constexpr static GLenum gs_type = GL_GEOMETRY_SHADER;
        constexpr static GLenum fs_type = GL_FRAGMENT_SHADER;

        // init by shader_desc
        std::string name{};
        // const char* vs{};
        // const char* gs{};
        // const char* fs{};
        vertex_layout layout;

        // then assign others
        GLuint gid{}; // program_id

        shader() = default;
        shader(const shader_desc& desc)
        {
            name = desc.name;
            layout = desc.layout;
        }
    };

    struct pipeline
    {
        // init by pipeline_desc
        std::string name{};

        GLenum primitive_type{ GL_TRIANGLES };
        GLenum cull_mode{ GL_BACK };

        // depth-stencil state
        bool depth_write_enabled{ false }; // ???
        GLenum depth_compare_func{ GL_ALWAYS };

        // then assign others


        pipeline() = default;
        pipeline(const pipeline_desc& desc)
        {
            name = desc.name;

            primitive_type = primitive_type_map.at(desc.primitive_type).opengl;

            cull_mode = cull_mode_map.at(desc.cull_mode).opengl;

            depth_write_enabled = desc.depth_write_enabled;
            depth_compare_func = compare_func_map.at(desc.depth_compare_func).opengl;
        }
    };

    struct frame
    {
        // init by frame_desc
        std::string name{};

        // then assign others
        GLuint gid{}; // program_id

        frame() = default;
        frame(const frame_desc& desc)
        {
            name = desc.name;
        }
    };
};

// TODO: move to pool.h
/*
1. class pool : pool_<buffer>, pool_<texture>...

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
struct resource_pool // pool
{
    std::array<uint32_t, 5> cnt{}; // unique_counter
    std::unordered_map<uint32_t, Buffer>   buffer_map;
    std::unordered_map<uint32_t, Texture>  texture_map;
    std::unordered_map<uint32_t, Shader>   shader_map;
    std::unordered_map<uint32_t, Pipeline> pipeline_map;
    std::unordered_map<uint32_t, Frame>    frame_map;

    buffer_id   insert(const   buffer_desc& desc) { uint32_t pid = ++cnt[0];   buffer_map.emplace(pid, desc); return   buffer_id(pid); }
    texture_id  insert(const  texture_desc& desc) { uint32_t pid = ++cnt[1];  texture_map.emplace(pid, desc); return  texture_id(pid); }
    shader_id   insert(const   shader_desc& desc) { uint32_t pid = ++cnt[2];   shader_map.emplace(pid, desc); return   shader_id(pid); }
    pipeline_id insert(const pipeline_desc& desc) { uint32_t pid = ++cnt[3]; pipeline_map.emplace(pid, desc); return pipeline_id(pid); }
    frame_id    insert(const    frame_desc& desc) { uint32_t pid = ++cnt[4];    frame_map.emplace(pid, desc); return    frame_id(pid); }

    Buffer&   operator[](  buffer_id key) { return   buffer_map[key.id]; }
    Texture&  operator[]( texture_id key) { return  texture_map[key.id]; }
    Shader&   operator[](  shader_id key) { return   shader_map[key.id]; }
    Pipeline& operator[](pipeline_id key) { return pipeline_map[key.id]; }
    Frame&    operator[](   frame_id key) { return    frame_map[key.id]; }

    // TODO: cache them
    // Or don't make this improvement, after all, maybe there's little performance boost.
    void erase(  buffer_id key) {   buffer_map.erase(key.id); }
    void erase( texture_id key) {  texture_map.erase(key.id); }
    void erase(  shader_id key) {   shader_map.erase(key.id); }
    void erase(pipeline_id key) { pipeline_map.erase(key.id); }
    void erase(   frame_id key) {    frame_map.erase(key.id); }
};

class device_opengl : public render_device, public opengl_utility
{
	// resource creation, updating and destruction
	buffer_id   create(const   buffer_desc& desc) override
	{
        // or id = buffer_pool.insert(besc), generate buffer by itself ???
        buffer_id pid = pool_.insert(desc); // id in the pool
        buffer& buf = pool_[pid];

        // TODO : dynamic, stream have more than one 

        glGenBuffers(1, &buf.gid); // vbo or ibo
        glBindBuffer(buf.type, buf.gid);
        glBufferData(buf.type, buf.size * buf.stride, nullptr, buf.usage);

        if (buf.usage == GL_STATIC_DRAW)
        {
            DCHECK(desc.data != nullptr);
            glBufferSubData(buf.type, 0, buf.size * buf.stride, desc.data);
        }
        glcheck_errors();

        return pid;
	}
	texture_id  create(const  texture_desc& desc) override
	{
        texture_id pid = pool_.insert(desc); // id in the pool
        texture& texture = pool_[pid];

        return pid;
	}
	shader_id   create(const   shader_desc& desc) override
	{
        shader_id pid = pool_.insert(desc); // id in the pool
        shader& shader = pool_[pid];

        std::vector<GLuint> ids; // shader_id

        ids.push_back(compile_shader(shader.name, shader::vs_type, desc.vs));
        ids.push_back(compile_shader(shader.name, shader::fs_type, desc.fs));

        if (desc.gs != nullptr)
            ids.push_back(compile_shader(shader.name, shader::gs_type, desc.gs));

        shader.gid = link_shader(shader.name, ids); // program_id
        glcheck_errors();

        return pid;
	}
	pipeline_id create(const pipeline_desc& desc) override
	{
        pipeline_id pid = pool_.insert(desc); // id in the pool
        //pipeline& pipeline = pool_[pid];

        // noop

        return pid;
	}
	frame_id    create(const    frame_desc& desc) override
	{
        frame_id pid = pool_.insert(desc); // id in the pool
        frame& frame = pool_[pid];



        return pid;
	}

	void update( buffer_id id, const void* data, int size) override
	{

	}
	void update(texture_id id, const void* data) override
	{

	}

	void destroy(  buffer_id id) override
	{
        auto gid = pool_[id].gid;
        // https://stackoverflow.com/questions/27937285/when-should-i-call-gldeletebuffers
        // void glDeleteBuffers(GLsizei n, const GLuint* buffers);
        // glDeleteBuffers(1, &gid);
        pool_.erase(id);
	}
	void destroy( texture_id id) override
	{
        auto gid = pool_[id].gid;
        pool_.erase(id);
	}
	void destroy(  shader_id id) override
	{
        auto gid = pool_[id].gid;
        // glDeleteProgram(gid);
        pool_.erase(id);
	}
	void destroy(pipeline_id id) override
	{
        pool_.erase(id);
	}
	void destroy(   frame_id id) override
	{
        auto gid = pool_[id].gid;
        pool_.erase(id);
	}

	// get resource state (initial, alloc, valid, failed)
	resource_state query_state(  buffer_id id) override
	{
        return resource_state();
	}
	resource_state query_state( texture_id id) override
	{
        return resource_state();
	}
	resource_state query_state(  shader_id id) override
	{
        return resource_state();
	}
	resource_state query_state(pipeline_id id) override
	{
        return resource_state();
	}
	resource_state query_state(   frame_id id) override
	{
        return resource_state();
	}

	// render	
    void draw(pass_desc desc) override
    {
        cache_.passes.emplace_back(std::move(desc));
        draw();
    }

	void submit(pass_desc desc) override
	{
        cache_.passes.emplace_back(std::move(desc));
	}

    void draw() override
    {
        // TODO: encode + sort
        for (const auto& pass : cache_.passes)
        {


            // check vertex layout

            // print stats

            // set vertex/instance buffer and vertex layout
            // TODO: cache vao
            for (int i = 0; i < Pass_max_buffers && pass.buffers[i].id != 0; ++i)
            {
                auto id = pass.buffers[i];
                auto buf = pool_[id];

                glBindBuffer(buf.type, buf.gid);
                for (int j = 0; j < buf.layout.size(); ++j)
                {
                    auto& a = buf.layout[i];
                    glEnableVertexAttribArray(j);
                    glVertexAttribPointer(j, a.size, a.type, a.normalized, buf.stride, a.offset);
                    if(buf.instance_rate > 0)
                        glVertexAttribDivisor(j, buf.instance_rate);
                }
            }
            glcheck_errors();

            // set texture

            // set shader
            auto shader = pool_[pass.shd_id];
            glUseProgram(shader.gid);
            glcheck_errors();

            // set pipeline
            auto pipe = pool_[pass.pipe_id];

            // set frame

            // clear
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // draw
            if (pass.index.id != 0)
            {
                // set index buffer
                auto index_buf = pool_[pass.index];
                glBindBuffer(index_buf.type, index_buf.gid);
                // TODO
                glDrawElements(pipe.primitive_type, index_buf.size, GL_UNSIGNED_INT, 0);
            }
            else
            {
                auto vertex_buf_id = pass.buffers[0];
                glDrawArrays(pipe.primitive_type, 0, pool_[vertex_buf_id].size);
            }
            glcheck_errors();
        }
        cache_.passes.clear();
    }

private:

    // create shader
    GLuint compile_shader(const std::string shader_name, GLenum type, const char* src)
    {
        auto id = glCreateShader(type);
        glShaderSource(id, 1, &src, NULL);
        glCompileShader(id);

        // check compile errors
        check_shader_errors(shader_name, id, type);
            
        return id;
    }
    GLuint link_shader(const std::string shader_name, std::vector<GLuint> ids)
    {
        GLuint program_id = glCreateProgram();

        for(auto id : ids)
            glAttachShader(program_id, id);

        glLinkProgram(program_id);

        // check link errors
        check_shader_errors(shader_name, program_id /*, 0*/);

        for (auto id : ids)
            glDeleteShader(id);

        return program_id;
    }
    void check_shader_errors(const std::string shader_name, GLuint id, GLenum type = 0)
    {
        static std::unordered_map<GLenum, const char*> name
        {
            { GL_VERTEX_SHADER,   "VERTEX_SHADER"   },
            { GL_GEOMETRY_SHADER, "GEOMETRY_SHADER" },
            { GL_FRAGMENT_SHADER, "FRAGMENT_SHADER" },
        };

        GLint state;
        constexpr GLsizei max_length = 1024;
        GLchar error_info[max_length];

        if (type != 0)
        {
            glGetShaderiv(id, GL_COMPILE_STATUS, &state);
            if (state == GL_FALSE)
            {
                glGetShaderInfoLog(id, max_length, NULL, error_info);
                LOG(ERROR) 
                    << "ERROR::SHADER_COMPILATION_ERROR" << "\n"
                    << "name: " << shader_name << " type: " << name[type] << "error: \n"
                    << error_info;
            }
        }
        else
        {
            glGetProgramiv(id, GL_LINK_STATUS, &state);
            if (state == GL_FALSE)
            {
                glGetProgramInfoLog(id, max_length, NULL, error_info);
                LOG(ERROR) 
                    << "ERROR::PROGRAM_LINKING_ERROR" << "\n"
                    << "name: " << shader_name << " type: " << "PROGRAM" << "error: \n"
                    << error_info;
            }
        }
    }

    // vertex attribure

    // 


private:
    // TODO: only change state when it isn't equal to context
	struct context
	{
        GLuint vao; // todo: comment
        GLuint framebuffer;

		//bool polygon_offset_enabled;
		//_gl_cache_attr attrs[MAX_VERTEX_ATTRIBUTES]{};    // {} fill false ???
		GLuint cur_gl_ib;
		uint32_t cur_ib_offset;
		GLenum cur_primitive_type;
		GLenum cur_index_type;
		//_pipeline* cur_pipeline;
		pipeline cur_pipeline;

		context() 
        { 
            //reset();

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);


        }
        ~context()
        {
            glDeleteVertexArrays(1, &vao);
        }

		void reset()
		{
			glcheck_errors();

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			glcheck_errors();

			/*
            for (int i = 0; i < MAX_VERTEX_ATTRIBUTES; i++) 
			{
				_gl_init_attr(&attrs[i].gl_attr);
				attrs[i].gl_vbuf = 0;
				glDisableVertexAttribArray(i);
				glcheck_errors();
			}
			cur_gl_ib = 0;
			cur_ib_offset = 0;
			cur_primitive_type = GL_TRIANGLES;
			cur_index_type = 0;
            */

			/* resource bindings */
			//cur_pipeline = 0;
			//cur_pipeline_id.id = INVALID_ID;

            //cur_pipeline = {}; // reset it
			/* depth-stencil state */
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glDepthMask(GL_FALSE);
			glDisable(GL_STENCIL_TEST);
			glStencilFunc(GL_ALWAYS, 0, 0);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glStencilMask(0);

			/* blend state */
			glDisable(GL_BLEND);
			glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);

			/* rasterizer state */
			//polygon_offset_enabled = false;
			glPolygonOffset(0.0f, 0.0f);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glDisable(GL_CULL_FACE);
			glFrontFace(GL_CW);
			glCullFace(GL_BACK);
			glEnable(GL_SCISSOR_TEST);
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			glEnable(GL_DITHER);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glEnable(GL_MULTISAMPLE);
			glEnable(GL_PROGRAM_POINT_SIZE);
		}
	};

    struct cache
    {
        std::vector<pass_desc> passes;

        cache() : passes(0) // TODO: default_pass_num
        {

        }
    };

    // TODO: multiwindow: one device + multictx   or   multidevice
    // one ctx bind to one window(multiwindow needs multicontext, multi-vao)
	context ctx_{};
    cache cache_{};
    resource_pool<buffer, texture, shader, pipeline, frame> pool_{};
};

// TODO: _ptr -> _up, _uptr _sp _wp
render_device_ptr create_device_opengl(const config& cfg)
{
    return std::make_unique<device_opengl>();
}

// class device_opengl_dsa : public render_device

} // namespace fay