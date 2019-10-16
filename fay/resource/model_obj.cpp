#include "fay/core/fay.h"
#include "fay/core/string.h"
#include "fay/render/define.h"
#include "fay/resource/file.h"
#include "fay/resource/model.h"
#include "fay/resource/text.h"

namespace fay
{

class obj_model : public resource_model
{
public:
    obj_model(const std::string& filepath, render_backend_type api);

private:

};

resource_model_ptr create_model_obj(const std::string& filepath, render_backend_type api)
{
    return std::make_unique<obj_model>(filepath, api);
}

struct obj_vertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texcoord{};

    obj_vertex(glm::vec3 p, glm::vec3 n, glm::vec2 t) :
        position(p),
        normal(n),
        texcoord(t)
    {
    }
};

struct obj_mesh
{
    std::string name{};
    std::string mat_name{};
    int smoothing_group{};

    std::vector<obj_vertex>  vertices{};
    std::vector<uint32_t> indices{};
};

struct obj_material
{
    std::string name{};

    size_t index_{}; // the Nth material, it's only used when converting

    float Ns, Ni, d, Tr, Tf;
    int illum;
    glm::vec3 Ka{};
    glm::vec3 Kd{};
    glm::vec3 Ks{};
    glm::vec3 Ke{};

    // static constexpr int nMap = 6;
    std::string map_Ka, map_Kd, map_Ks, map_Ke, map_d, map_bump;
    // std::vector<uint32_t> sub_indices;
    // int offset;
    // int count;

    // obj_material() { memset(this, 0, sizeof(obj_material)); }
};

enum class obj_keyword
{
    dummy,
    comment, // '#'
    v, vn, vt,
    o, g, s, f, // TODO: object, group, smooth, face
    mtllib, usemtl, newmtl,
    Ns, Ni, d, Tr, Tf, illum,
    Ka, Kd, Ks, Ke,
    map_Ka, map_Kd, map_Ks, map_Ke, map_d, map_bump
};

static const std::unordered_map<std::string, obj_keyword> keyword
{
    { "#", obj_keyword::comment },

    { "v",  obj_keyword::v },
    { "vn", obj_keyword::vn },
    { "vt", obj_keyword::vt },

    { "o", obj_keyword::o },
    { "g", obj_keyword::g },
    { "s", obj_keyword::s },
    { "f", obj_keyword::f },

    { "mtllib", obj_keyword::mtllib },
    { "usemtl", obj_keyword::usemtl },
    { "newmtl", obj_keyword::newmtl },

    { "Ns", obj_keyword::Ns },
    { "Ni", obj_keyword::Ni },
    { "d",  obj_keyword::d },
    { "Tr", obj_keyword::Tr },
    { "Tf", obj_keyword::Tf },
    { "illum", obj_keyword::illum },

    { "Ka", obj_keyword::Ka },
    { "Kd", obj_keyword::Kd },
    { "Ks", obj_keyword::Ks },
    { "Ke", obj_keyword::Ke },

    // TODO: rename obj_keyword::map_d to obj_keyword::map_alpha
    { "map_Ka", obj_keyword::map_Ka },
    { "map_Kd", obj_keyword::map_Kd },
    { "map_Ks", obj_keyword::map_Ks },
    { "map_Ke", obj_keyword::map_Ke },
    { "map_d",  obj_keyword::map_d },

    // TODO: rename obj_keyword::map_bump to obj_keyword::map_normal
    { "bump",     obj_keyword::map_bump },
    { "map_bump", obj_keyword::map_bump },
    { "map_Bump", obj_keyword::map_bump },

    { "alpha_test", obj_keyword::dummy },
};

static std::vector<obj_mesh> load_meshs(const std::string& firstline, std::ifstream& file, bool face_winding_ccw);

static std::unordered_map<std::string, obj_material> load_materials(const std::string& directory, const std::string& filename);

// TODO: boost::format
obj_model::obj_model(const std::string& filepath, render_backend_type api) : resource_model(filepath, api)
{
    bool flip_vertical = (api == render_backend_type::opengl);
    bool face_winding_ccw = (api == render_backend_type::opengl); // TODO
    auto directory = get_directory(filepath);

    // load *.obj
    auto file = load_text(filepath);

    std::cout << "loading the file: " << filepath << '\n';

    std::unordered_map<std::string, obj_material> objmaterials;
    std::vector<obj_mesh> submeshes; // TODO: remove
    std::vector<obj_mesh> objmeshes;

    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        line = erase_white_spaces(line);
        if (line.length() <= 2) continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        CHECK(keyword.find(token) != keyword.end()) << "can't parser the token: " << token;

        switch (keyword.at(token))	// operator[] only for nonconstant 
        {
            case obj_keyword::comment:
                std::cout << "obj file comment: " << line << '\n';
                break;

            case obj_keyword::mtllib:
                // parse *.mtl
                objmaterials = load_materials(directory, erase_front_word(line));
                break;

            case obj_keyword::o:
            case obj_keyword::v:
                // CHECK(!line.empty()) << "can't parser the line: " << line;
                submeshes = load_meshs(line, file, face_winding_ccw);
                objmeshes.insert(objmeshes.end(), submeshes.begin(), submeshes.end());
                break;

            default:
                LOG(ERROR) << "can't parser the token: " << token;
                break;
        }
    }

    // convert obj_mesh, obj_mat to resource_mesh, mat

    materials_.reserve(objmaterials.size());
    meshes_.reserve(objmeshes.size());

    size_t mat_index{};
    for (auto& [mat_name, objmat] : objmaterials)
    {
        objmat.index_ = mat_index++;

        resource_material mat;

        mat.name = objmat.name;
        mat.base_factor = glm::vec4{ objmat.Kd, 1.f }; // objmat.Tr

        if (!objmat.map_Kd.empty()) // ;
            mat.base_color = image(directory + objmat.map_Kd, flip_vertical); // base_color with alpha_mask
        
        mat.metallic_roughness = convert_to_metallic_roughness(directory, objmat.map_Ka, objmat.map_Ks, flip_vertical);

        if (!objmat.map_bump.empty())
            mat.normal = image(directory + objmat.map_bump, flip_vertical);

        materials_.push_back(std::move(mat));
    }

    for (auto& objmesh : objmeshes)
    {
        resource_mesh mesh;

        mesh.name = objmesh.name;

        mesh.size = objmesh.vertices.size();
        mesh.layout = vertex_layout{
            {fay::attribute_usage::position,  fay::attribute_format::float3},
            {fay::attribute_usage::normal,    fay::attribute_format::float3},
            {fay::attribute_usage::texcoord0, fay::attribute_format::float2}
        };
        DCHECK(mesh.layout.stride() == 32);

        size_t byte_size = mesh.size * mesh.layout.stride();
        mesh.vertices.reserve(byte_size);
        mesh.vertices.resize(byte_size);
        std::memcpy(mesh.vertices.data(), objmesh.vertices.data(), byte_size);

        mesh.indices = objmesh.indices;
        mesh.material_index = objmaterials[objmesh.mat_name].index_;

        meshes_.push_back(std::move(mesh));
    }

    // set scenes, nodes
    //default_scene_.name = get_filename(filepath);
    //default_scene_.nodes.push_back(0);
    //scenes_.push_back(default_scene_);

    nodes_.resize(meshes_.size());
    for (auto i : range(meshes_.size()))
        nodes_[i].meshes.push_back(i);

    root_node_.children.resize(nodes_.size());
    std::iota(root_node_.children.begin(), root_node_.children.end(), 1);

    nodes_.insert(nodes_.begin(), root_node_);
}

static std::vector<obj_mesh> load_meshs(const std::string& firstline, std::ifstream& file, bool face_winding_ccw) // face_winding, Counter-ClockWise order in gl
{
    std::vector<obj_mesh> submeshes;

    obj_mesh mesh;
    std::string cur_mesh_name;

    // v, vn, vt
    // in obj model, index starting from 1, like "f 1/1/1 2/2/1 3/3/1 4/4/1"
    std::vector<glm::vec3> positions(1, glm::vec3());
    std::vector<glm::vec3> normals(1, glm::vec3());
    std::vector<glm::vec3> texcoords(1, glm::vec3()); // TODO:??? vec2

    // int num;

    glm::vec3 v{};	// value

    // TODO: get_xyz and return vec3(not cache it)
    auto get_xyz = [&v](std::istringstream& iss) { iss >> v.x >> v.y >> v.z; };

    auto add_and_clear = [&submeshes](obj_mesh& mesh)
    {
        if (!mesh.vertices.empty() && !mesh.indices.empty())
        {	// ±ÜÃâ¼ÓÈë¿ÕµÄ mesh
            submeshes.push_back(std::move(mesh));
            mesh = obj_mesh();	// clear
        }
    };

    // TODO: remove firstline
    if (firstline[0] == 'o')
        mesh.name = erase_front_word(firstline);
    else
    {
        std::istringstream iss(firstline);
        std::string token;
        iss >> token >> v.x >> v.y >> v.z;
        positions.emplace_back(v);
    }

    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        line = erase_white_spaces(line);
        if (line.length() <= 2) continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        CHECK(keyword.find(token) != keyword.end()) << "can't parser the token: " << token;

        switch (keyword.at(token))	// operator[] only for nonconstant 
        {
        // TODO: v、vn、vt、f......
        // TODO: simplify case f
            case obj_keyword::comment:

                std::cout << "obj file comment: " << line << '\n';
                break;

            case obj_keyword::v:

                get_xyz(iss);
                positions.emplace_back(v);
                break;

            case obj_keyword::vn:

                get_xyz(iss);
                normals.emplace_back(v);
                break;

            case obj_keyword::vt:

                get_xyz(iss);
                texcoords.emplace_back(v);
                break;

            case obj_keyword::o:	// firstline
            case obj_keyword::g:

                // a new submesh
                add_and_clear(mesh);
                iss >> cur_mesh_name;
                mesh.name = cur_mesh_name;
                break;

            case obj_keyword::usemtl:

                add_and_clear(mesh);
                iss >> mesh.mat_name;
                mesh.name = cur_mesh_name + '_' + mesh.mat_name;
                break;

            case obj_keyword::s:

                iss >> mesh.smoothing_group;
                break;

            case obj_keyword::f:
            {
                // TODO: more simple way
                auto num_of_char = [](const std::string& line, char ch)
                {
                    size_t count = 0, pos = line.find(ch);
                    while (pos != std::string::npos)
                    {
                        ++count;
                        pos = line.find(ch, pos + 1);
                    }
                    return count;
                };

                glm::ivec4 p{}, t{}, n{};	// index

                int count = 3;	// vertex count of face, 3(triangle) or 4(rectangle)
                if (num_of_char(line, ' ') == 4)
                    count = 4;

                char ch;

                if (line.find("//") != std::string::npos)
                {	// pos//normal, no uv. "f 181//176 182//182 209//208"

                    for (int i = 0; i < count; ++i)
                        iss >> p[i] >> ch >> ch >> n[i];
                }
                else
                {
                    auto nSprit = num_of_char(line, '/');

                    if ((nSprit == 6) || (nSprit == 8))
                    {	// pos/uv/normal. "f 181/292/176 182/250/182 209/210/208"

                        for (int i = 0; i < count; ++i)
                            iss >> p[i] >> ch >> t[i] >> ch >> n[i];
                    }
                    else if ((nSprit == 3) || (nSprit == 4))
                    {	// pos/uv, no normal. "f 181/176 182/182 209/208"

                        for (int i = 0; i < count; ++i)
                            iss >> p[i] >> ch >> t[i];
                    }
                    else
                    {	// pos, no uv/normal. "f 181 182 209"

                        for (int i = 0; i < count; ++i)
                            iss >> p[i];
                    }
                }

                // deal with negative index
                // come from -1
                auto deal_with_negative_index = [](glm::ivec4& v, size_t sz)
                {
                    if (v[0] < 0) { /*v += 1;*/ v += sz; }
                };

                deal_with_negative_index(p, positions.size());
                deal_with_negative_index(t, texcoords.size());
                deal_with_negative_index(n, normals.size());

                // vertex
                glm::vec3 pos, nor, tex;
                // index, starts from 0
                uint32_t index = mesh.vertices.size();

                for (int i = 0; i < count; ++i)
                {	// why access container[0] is ok???
                    // TODO: can't access container[0].
                    DCHECK(0 <= p[i] && p[i] < positions.size()) << "vector out of range: " << p[i];
                    DCHECK(0 <= n[i] && n[i] < normals.size())   << "vector out of range: " << n[i];
                    DCHECK(0 <= t[i] && t[i] < texcoords.size()) << "vector out of range: " << t[i];
                    pos = positions[p[i]];
                    nor = normals[n[i]];
                    tex = texcoords[t[i]];
                    mesh.vertices.emplace_back(pos, nor, glm::vec2{ tex.s, tex.t });
                }

                // index
                if (face_winding_ccw)
                {
                    mesh.indices.insert(mesh.indices.end(),
                        { index, index + 1, index + 2 });

                    if (count == 4)
                        mesh.indices.insert(mesh.indices.end(),
                            { index + 2, index + 3, index });
                }
                else
                {
                    mesh.indices.insert(mesh.indices.end(),
                        { index + 2, index + 1, index });

                    if (count == 4)
                        mesh.indices.insert(mesh.indices.end(),
                            { index, index + 3, index + 2 });
                }

                break;
            }

            default:
                LOG(ERROR) << "can't parser the token: " << token;
                break;
        }
    }
    add_and_clear(mesh);	// ¼ÓÈë×îºóÒ»¸ö mesh

    return std::move(submeshes);
}

static std::unordered_map<std::string, obj_material> load_materials(const std::string& directory, const std::string& filename)
{
    // load *.mtl
    auto file = load_text(directory + filename);
    std::unordered_map<std::string, obj_material> materials;
    obj_material mat;

    glm::vec3 v{};	// value

    auto get_xyz = [&v](std::istringstream& iss) { iss >> v.x >> v.y >> v.z; };

    auto add_and_clear = [&materials](obj_material& mat)
    {
        if (!mat.name.empty())
        {
            materials.insert({ mat.name, std::move(mat) });
            mat = obj_material(); // clear
        }
    };

    while (!file.eof())
    {
        std::string line;
        std::getline(file, line);
        line = erase_white_spaces(line);
        if (line.length() <= 2) continue;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        CHECK(keyword.find(token) != keyword.end()) << "can't parser the token: " << token;

        switch (keyword.at(token))	// operator[] only for nonconstant 
        {
            case obj_keyword::comment:
                std::cout << "mtl file comment: " << line << '\n';
                break;

            case obj_keyword::newmtl:
                add_and_clear(mat);
                iss >> mat.name;
                break;

            case obj_keyword::Ns: iss >> mat.Ns; break;
            case obj_keyword::Ni: iss >> mat.Ni; break;
            case obj_keyword::d: iss >> mat.d;  break;
            case obj_keyword::Tr: iss >> mat.Tr; break;
            case obj_keyword::Tf: iss >> mat.Tf; break;
            case obj_keyword::illum: iss >> mat.illum; break;

            case obj_keyword::Ka: get_xyz(iss); mat.Ka = v; break;
            case obj_keyword::Kd: get_xyz(iss); mat.Kd = v; break;
            case obj_keyword::Ks: get_xyz(iss); mat.Ks = v; break;
            case obj_keyword::Ke: get_xyz(iss); mat.Ke = v; break;

            case obj_keyword::map_Ka: iss >> mat.map_Ka; break;
            case obj_keyword::map_Kd: iss >> mat.map_Kd; break;
            case obj_keyword::map_Ks: iss >> mat.map_Ks; break;
            case obj_keyword::map_Ke: iss >> mat.map_Ke; break;
            case obj_keyword::map_d: iss >> mat.map_d;  break;
            case obj_keyword::map_bump: iss >> mat.map_bump; break;

            case obj_keyword::dummy:
            default:
                LOG(ERROR) << "can't parser the token: " << token;
                break;
        }
    }
    add_and_clear(mat);

    return std::move(materials);
}

/*
void objMesh_transform_to_TextureDataArray(
    std::vector<obj_mesh>& meshes,
    std::vector<glm::vec4>& positions,
    std::vector<glm::uvec4>& indices,
    std::vector<std::string>& texpaths)
{
    // ¿¼ÂÇµ½ºÏ²¢ÁËÖØ¸´µÄÎÆÀí£¬Òò´Ë texpaths ¿ÉÄÜ¸üÐ¡Ò»µã
    // positions[p1, p2, ...... pn]
    // indices  [i1, i2, ...... in]
    // texpaths [t1, t2, ...... tn]

    positions.clear(); // positions.reserve(mesh.size())
    indices.clear();
    for (auto& mesh : meshes)
    {
        for (auto& vertex : mesh.vertices)
            positions.emplace_back(glm::vec4{ vertex.position, 0.f });

        uint32_t tex_index{};
        if (!mesh.images.empty())
        {
            // Ö»Ê¹ÓÃµÚÒ»ÕÅÎÆÀí
            std::string tex_filepath = { mesh.images[0].first.filepath };

            if (std::find(texpaths.begin(), texpaths.end(), tex_filepath) == texpaths.end())
                texpaths.push_back(tex_filepath);

            for (; tex_index < texpaths.size(); ++tex_index)
                if (texpaths[tex_index] == tex_filepath)
                    break;
        }
        else
            tex_index = 255;

        // ÔÚ×ÅÉ«Æ÷ÖÐÍ¨¹ý tex_index£¬¼´ÎÆÀíµÄÏÂ±ê£¬ÕÒµ½¶ÔÓ¦µÄÎÆÀí
        auto& indices = mesh.indices;
        for (size_t j{}; j < indices.size(); j += 3)
            indices.emplace_back(glm::vec4{ indices[j], indices[j + 1], indices[j + 2], tex_index });
    }
    LOG(INFO) << " transform_to_TextureDataArray: "
        << " positions: " << positions.size()
        << " indices: " << indices.size()
        << " texpaths: " << texpaths.size();
}
*/

} // namespace fay