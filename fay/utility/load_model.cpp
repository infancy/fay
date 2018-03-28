#include "load_model.h"
#include "fay/utility/file.h"
#include "fay/utility/string.h"
#include <assimp/Importer.hpp>

// using namespace std;

namespace fay
{

resource_model::resource_model(const std::string& filepath, third_party api) : 
	path{ get_path(filepath) }, api{ api } {}

// load obj model --------------------------------------------------------------

static const std::unordered_map<std::string, obj_keyword> map
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

	{ "map_Ka", obj_keyword::map_Ka },
	{ "map_Kd", obj_keyword::map_Kd },
	{ "map_Ks", obj_keyword::map_Ks },
	{ "map_Ke", obj_keyword::map_Ke },
	{ "map_d", obj_keyword::map_d },
	{ "map_bump", obj_keyword::map_bump },
	{ "map_Bump", obj_keyword::map_bump },
};

// boost::format
// 理论上一个 mesh 由 'o' 开始，但碰到 'o'，'g'，'usemtl'，就新建一个 mesh
obj_model::obj_model(const std::string& filepath, third_party api) : resource_model(filepath, api)
{
	// load *.obj
	std::ifstream file(filepath);
	CHECK(!file.fail()) << "can't open the file: " << filepath;

	std::cout << "loading the file: " << filepath << '\n';

	std::unordered_map<std::string, obj_material> materials;
	std::vector<obj_mesh> submeshes;
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

		CHECK(map.find(token) != map.end()) << "can't parser the token: " << token;

		switch (map.at(token))	// operator[] only for nonconstant 
		{
		case obj_keyword::comment :
			std::cout << "obj file comment: " << line << '\n';
			break;

		case obj_keyword::mtllib :
			// std::fstream mtl("*.mtl");
			materials = load_materials(erase_front_word(line));
			break;

		case obj_keyword::o :
		case obj_keyword::v :
			// CHECK(!line.empty()) << "can't parser the line: " << line;
			submeshes = load_meshs(line, file);
			objmeshes.insert(objmeshes.end(), submeshes.begin(), submeshes.end());
			break;

		default:
			LOG(ERROR) << "can't parser the token: " << token;
			break;
		}
	}

	for (auto& mesh : objmeshes)
		meshes.push_back({ std::move(mesh), materials[mesh.mat_name] });
		// std::move(materials[mesh.mat_name]) });
}

std::vector<obj_mesh> obj_model::load_meshs(
	const std::string& firstline, std::ifstream& file)
{
	std::vector<obj_mesh> submeshes;

	obj_mesh mesh;
	std::string cur_mesh_name;

	// TODO: explain
	// std::vector<glm::vec3> positions{ 0 };
	std::vector<glm::vec3> positions(1, glm::vec3());
	std::vector<glm::vec3> normals(1, glm::vec3());
	std::vector<glm::vec3> texcoords(1, glm::vec3());

	// int nPositions{}, nNormals{}, nTexcoods{};

	glm::vec3 v{};	// value

	auto get_xyz = [&v](std::istringstream& iss) { iss >> v.x >> v.y >> v.z; };
	
	auto add_and_clear = [&submeshes](obj_mesh& mesh) 
	{ 
		if(!mesh.vertices.empty() && !mesh.indices.empty())
		{	// 避免加入空的 mesh
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

		CHECK(map.find(token) != map.end()) << "can't parser the token: " << token;

		switch (map.at(token))	// operator[] only for nonconstant 
		{
		case obj_keyword::comment:
			std::cout << "obj file comment: " << line << '\n';
			break;

		case obj_keyword::v :
			get_xyz(iss);
			positions.emplace_back(v);
			break;
		case obj_keyword::vn :
			get_xyz(iss);
			normals.emplace_back(v);
			break;
		case obj_keyword::vt :
			get_xyz(iss);
			texcoords.emplace_back(v);
			break;

		case obj_keyword::o :	// firstline
		case obj_keyword::g :
			add_and_clear(mesh);
			iss >> cur_mesh_name;
			mesh.name = cur_mesh_name;
			break;

		case obj_keyword::usemtl:
			add_and_clear(mesh);
			iss >> mesh.mat_name;
			mesh.name = cur_mesh_name + '_' + mesh.mat_name;
			break;

		case obj_keyword::s :
			iss >> mesh.smoothing_group;
			break;

		case obj_keyword::f :
		{
			// TODO: more simple way
			auto num_of_char = [](const std::string& line, char ch)-> size_t
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

			int count = 3;	// 若一个 face 有四个顶点，则需要拆分
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
			// uint32_t index = mesh.vertices.size() - 1;
			uint32_t index = mesh.vertices.size();

			for(int i = 0; i < count; ++i)
			{	// access container[0] is ok
				DCHECK(0 <= p[i] && p[i] < positions.size()) << "vector out of range: " << p[i];
				DCHECK(0 <= n[i] && n[i] < normals.size())   << "vector out of range: " << n[i];
				DCHECK(0 <= t[i] && t[i] < texcoords.size()) << "vector out of range: " << t[i];
				pos = positions[p[i]];
				nor   = normals[n[i]];
				tex = texcoords[t[i]];
				mesh.vertices.emplace_back(pos, nor, glm::vec2{tex.s, tex.t});
			}

			// index
			if (api == third_party::gl)
			{   // 此时无需 UV 反转
				mesh.indices.insert(mesh.indices.end(), 
					{ index, index + 1, index + 2 });

				if (count == 4)
					mesh.indices.insert(mesh.indices.end(),
						{ index + 2, index + 3, index });
				/*
				mesh.indices.insert(mesh.indices.end(), 
					{ index + 2, index + 1, index });

				if (count == 4)
					mesh.indices.insert(mesh.indices.end(),
						{ index, index + 3, index + 2 });
				*/
			}

			break;
		}

		default:
			LOG(ERROR) << "can't parser the token: " << token;
			break;
		}
	}
	add_and_clear(mesh);	// 加入最后一个 mesh

	return std::move(submeshes);
}

std::unordered_map<std::string, obj_material> 
obj_model::load_materials(const std::string& filepath)
{
	// load *.mtl
	auto file = load_file(this->path + filepath);
	std::unordered_map<std::string, obj_material> materials;
	obj_material mat;

	glm::vec3 v{};	// value

	auto get_xyz = [&v](std::istringstream& iss) { iss >> v.x >> v.y >> v.z; };

	auto add_and_clear = [&materials](obj_material& mat)
	{
		if (!mat.name.empty())
		{	// 避免加入空的 material
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

		CHECK(map.find(token) != map.end()) << "can't parser the token: " << token;

		switch (map.at(token))	// operator[] only for nonconstant 
		{
		case obj_keyword::comment :
			std::cout << "mtl file comment: " << line << '\n';
			break;

		case obj_keyword::newmtl :
			add_and_clear(mat);
			iss >> mat.name;
			break;

		case obj_keyword::Ns : iss >> mat.Ns; break;
		case obj_keyword::Ni : iss >> mat.Ni; break;
		case obj_keyword::d  : iss >> mat.d;  break;
		case obj_keyword::Tr : iss >> mat.Tr; break;
		case obj_keyword::Tf : iss >> mat.Tf; break;
		case obj_keyword::illum : iss >> mat.illum; break;

		case obj_keyword::Ka : get_xyz(iss); mat.Ka = v; break;
		case obj_keyword::Kd : get_xyz(iss); mat.Kd = v; break;
		case obj_keyword::Ks : get_xyz(iss); mat.Ks = v; break;
		case obj_keyword::Ke : get_xyz(iss); mat.Ke = v; break;

		case obj_keyword::map_Ka : iss >> mat.map_Ka; break;
		case obj_keyword::map_Kd : iss >> mat.map_Kd; break;
		case obj_keyword::map_Ks : iss >> mat.map_Ks; break;
		case obj_keyword::map_Ke : iss >> mat.map_Ke; break;
		case obj_keyword::map_d  : iss >> mat.map_d;  break;
		case obj_keyword::map_bump : iss >> mat.map_bump; break;

		default:
			LOG(ERROR) << "can't parser the token: " << token;
			break;
		}
	}
	add_and_clear(mat);
	
	return std::move(materials);
}

// 将位置、索引转换成纹理数据
/*
void objMesh_transform_to_TextureDataArray(
	std::vector<obj_mesh>& meshes,
	std::vector<glm::vec4>& positions,
	std::vector<glm::uvec4>& indices,
	std::vector<std::string>& texpaths)
{
	// 考虑到合并了重复的纹理，因此 texpaths 可能更小一点
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
			// 只使用第一张纹理
			std::string tex_filepath = { mesh.images[0].first.filepath };

			if (std::find(texpaths.begin(), texpaths.end(), tex_filepath) == texpaths.end())
				texpaths.push_back(tex_filepath);

			for (; tex_index < texpaths.size(); ++tex_index)
				if (texpaths[tex_index] == tex_filepath)
					break;
		}
		else
			tex_index = 255;

		// 在着色器中通过 tex_index，即纹理的下标，找到对应的纹理
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

// load model by assimp --------------------------------------------------------

assimp_model::assimp_model(const std::string& filepath, third_party api, model_type model_type) : 
	resource_model(filepath, api), modeltype{ model_type }
{
	Assimp::Importer importer;
	const aiScene* scene{};
	if(api == third_party::gl)
		scene = importer.ReadFile(filepath,
			aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	else
		scene = importer.ReadFile(filepath,
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		LOG(ERROR) << "ASSIMP:: " << importer.GetErrorString();
		return;
	}

	process_node(scene->mRootNode, scene);	// process ASSIMP's root node recursively
}

void assimp_model::process_node(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene);
}

assimp_mesh assimp_model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<vertex5>  vertices;
	std::vector<uint32_t> indices;
	std::vector<std::pair<image_ptr, texture_type>> images;

	// vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		vertex5 vertex;

		auto copy = [](glm::vec3& vec, aiVector3D& rhs) { vec.x = rhs.x; vec.y = rhs.y; vec.z = rhs.z; };

		copy(vertex.position, mesh->mVertices[i]);
		copy(vertex.normal, mesh->mNormals[i]);
		if(modeltype == model_type::obj)
		{ 
			copy(vertex.tangent, mesh->mTangents[i]);
			copy(vertex.bitangent, mesh->mBitangents[i]);
		}

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vertex.texcoord.x = mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
			vertex.texcoord = glm::vec2(0.f, 0.f);

		vertices.push_back(vertex);
	}

	// indices
	for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (uint32_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	auto load_maps = [this, &images, material](aiTextureType type, texture_type textype)
	{
		auto maps = load_images(material, type, textype);
		images.insert(images.end(), maps.begin(), maps.end());
	};

	load_maps(aiTextureType_AMBIENT,  texture_type::ambient);
	load_maps(aiTextureType_DIFFUSE,  texture_type::diffuse);
	load_maps(aiTextureType_SPECULAR, texture_type::specular);
	if (modeltype == model_type::obj)
		load_maps(aiTextureType_HEIGHT,   texture_type::parallax);
	else
		load_maps(aiTextureType_NORMALS,  texture_type::parallax);
	//std::cout << "\nnormals:";
	//load_maps(aiTextureType_NORMALS,  texture_type::normals);
	//load_maps(aiTextureType_UNKNOWN, texture_type::unknown);

	return assimp_mesh(vertices, indices, images);
}

std::vector<std::pair<image_ptr, texture_type>> 
assimp_model::load_images(aiMaterial* mat, aiTextureType type, texture_type textype)
{
	std::vector<std::pair<image_ptr, texture_type>> images;

	for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString ainame;
		mat->GetTexture(type, i, &ainame);
		std::string name{ ainame.C_Str() };

		if(!name.empty())
		{ 
			if (images_cache.find(name) == images_cache.end())
			{
				std::cout << "\nload image: " << (path + name);
				image_ptr img(path + name, api);
				images.push_back({img, textype});
				images_cache.insert({ name, img });
			}
			else
				images.push_back({images_cache[name], textype});
		}
	}
	return std::move(images);
}

} // namespace fay