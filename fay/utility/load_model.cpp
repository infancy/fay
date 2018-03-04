#include "load_model.h"
#include "fay/utility/file.h"
#include "fay/utility/string.h"
#include <assimp/Importer.hpp>

// using namespace std;

namespace fay
{

BaseModel::BaseModel(const std::string& filepath, Thirdparty api) : 
	path{ get_path(filepath) }, api{ api } {}

// load obj model --------------------------------------------------------------

static const std::unordered_map<std::string, ObjKeyword> map
{
	{ "#", ObjKeyword::comment },

	{ "v",  ObjKeyword::v },
	{ "vn", ObjKeyword::vn },
	{ "vt", ObjKeyword::vt },

	{ "o", ObjKeyword::o },
	{ "g", ObjKeyword::g },
	{ "s", ObjKeyword::s },
	{ "f", ObjKeyword::f },

	{ "mtllib", ObjKeyword::mtllib },
	{ "usemtl", ObjKeyword::usemtl },
	{ "newmtl", ObjKeyword::newmtl },

	{ "Ns", ObjKeyword::Ns },
	{ "Ni", ObjKeyword::Ni },
	{ "d",  ObjKeyword::d },
	{ "Tr", ObjKeyword::Tr },
	{ "Tf", ObjKeyword::Tf },
	{ "illum", ObjKeyword::illum },

	{ "Ka", ObjKeyword::Ka },
	{ "Kd", ObjKeyword::Kd },
	{ "Ks", ObjKeyword::Ks },
	{ "Ke", ObjKeyword::Ke },

	{ "map_Ka", ObjKeyword::map_Ka },
	{ "map_Kd", ObjKeyword::map_Kd },
	{ "map_Ks", ObjKeyword::map_Ks },
	{ "map_Ke", ObjKeyword::map_Ke },
	{ "map_d", ObjKeyword::map_d },
	{ "map_bump", ObjKeyword::map_bump },
	{ "map_Bump", ObjKeyword::map_bump },
};

// boost::format
// 理论上一个 mesh 由 'o' 开始，但碰到 'o'，'g'，'usemtl'，就新建一个 mesh
ObjModel::ObjModel(const std::string& filepath, Thirdparty api) : BaseModel(filepath, api)
{
	// load *.obj
	std::ifstream file(filepath);
	CHECK(!file.fail()) << "can't open the file: " << filepath;

	std::cout << "loading the file: " << filepath << '\n';

	std::unordered_map<std::string, ObjMaterial> materials;
	std::vector<ObjMesh> submeshes;
	std::vector<ObjMesh> objmeshes;

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
		case ObjKeyword::comment :
			std::cout << "obj file comment: " << line << '\n';
			break;

		case ObjKeyword::mtllib :
			// std::fstream mtl("*.mtl");
			materials = load_materials(erase_front_word(line));
			break;

		case ObjKeyword::o :
		case ObjKeyword::v :
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

std::vector<ObjMesh> ObjModel::load_meshs(
	const std::string& firstline, std::ifstream& file)
{
	std::vector<ObjMesh> submeshes;

	ObjMesh mesh;
	std::string cur_mesh_name;

	// TODO: explain
	// std::vector<glm::vec3> positions{ 0 };
	std::vector<glm::vec3> positions(1, glm::vec3());
	std::vector<glm::vec3> normals(1, glm::vec3());
	std::vector<glm::vec3> texcoords(1, glm::vec3());

	// int nPositions{}, nNormals{}, nTexcoods{};

	glm::vec3 v{};	// value

	auto get_xyz = [&v](std::istringstream& iss) { iss >> v.x >> v.y >> v.z; };
	
	auto add_and_clear = [&submeshes](ObjMesh& mesh) 
	{ 
		if(!mesh.vertices.empty() && !mesh.indices.empty())
		{	// 避免加入空的 mesh
			submeshes.push_back(std::move(mesh));
			mesh = ObjMesh();	// clear
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
		case ObjKeyword::comment:
			std::cout << "obj file comment: " << line << '\n';
			break;

		case ObjKeyword::v :
			get_xyz(iss);
			positions.emplace_back(v);
			break;
		case ObjKeyword::vn :
			get_xyz(iss);
			normals.emplace_back(v);
			break;
		case ObjKeyword::vt :
			get_xyz(iss);
			texcoords.emplace_back(v);
			break;

		case ObjKeyword::o :	// firstline
		case ObjKeyword::g :
			add_and_clear(mesh);
			iss >> cur_mesh_name;
			mesh.name = cur_mesh_name;
			break;

		case ObjKeyword::usemtl:
			add_and_clear(mesh);
			iss >> mesh.mat_name;
			mesh.name = cur_mesh_name + '_' + mesh.mat_name;
			break;

		case ObjKeyword::s :
			iss >> mesh.smoothing_group;
			break;

		case ObjKeyword::f :
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
			if (api == Thirdparty::gl)
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

std::unordered_map<std::string, ObjMaterial> 
ObjModel::load_materials(const std::string& filepath)
{
	// load *.mtl
	auto file = load_file(this->path + filepath);
	std::unordered_map<std::string, ObjMaterial> materials;
	ObjMaterial mat;

	glm::vec3 v{};	// value

	auto get_xyz = [&v](std::istringstream& iss) { iss >> v.x >> v.y >> v.z; };

	auto add_and_clear = [&materials](ObjMaterial& mat)
	{
		if (!mat.name.empty())
		{	// 避免加入空的 material
			materials.insert({ mat.name, std::move(mat) });
			mat = ObjMaterial(); // clear
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
		case ObjKeyword::comment :
			std::cout << "mtl file comment: " << line << '\n';
			break;

		case ObjKeyword::newmtl :
			add_and_clear(mat);
			iss >> mat.name;
			break;

		case ObjKeyword::Ns : iss >> mat.Ns; break;
		case ObjKeyword::Ni : iss >> mat.Ni; break;
		case ObjKeyword::d  : iss >> mat.d;  break;
		case ObjKeyword::Tr : iss >> mat.Tr; break;
		case ObjKeyword::Tf : iss >> mat.Tf; break;
		case ObjKeyword::illum : iss >> mat.illum; break;

		case ObjKeyword::Ka : get_xyz(iss); mat.Ka = v; break;
		case ObjKeyword::Kd : get_xyz(iss); mat.Kd = v; break;
		case ObjKeyword::Ks : get_xyz(iss); mat.Ks = v; break;
		case ObjKeyword::Ke : get_xyz(iss); mat.Ke = v; break;

		case ObjKeyword::map_Ka : iss >> mat.map_Ka; break;
		case ObjKeyword::map_Kd : iss >> mat.map_Kd; break;
		case ObjKeyword::map_Ks : iss >> mat.map_Ks; break;
		case ObjKeyword::map_Ke : iss >> mat.map_Ke; break;
		case ObjKeyword::map_d  : iss >> mat.map_d;  break;
		case ObjKeyword::map_bump : iss >> mat.map_bump; break;

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
	std::vector<ObjMesh>& meshes,
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

AssimpModel::AssimpModel(const std::string& filepath, Thirdparty api, ModelType model_type) : 
	BaseModel(filepath, api), model_type{ model_type }
{
	Assimp::Importer importer;
	const aiScene* scene{};
	if(api == Thirdparty::gl)
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

void AssimpModel::process_node(aiNode* node, const aiScene* scene)
{
	for (uint32_t i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(process_mesh(mesh, scene));
	}

	for (uint32_t i = 0; i < node->mNumChildren; ++i)
		process_node(node->mChildren[i], scene);
}

AssimpMesh AssimpModel::process_mesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex5>  vertices;
	std::vector<uint32_t> indices;
	std::vector<std::pair<Image, TexType>> images;

	// vertices
	for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex5 vertex;

		auto copy = [](glm::vec3& vec, aiVector3D& rhs) { vec.x = rhs.x; vec.y = rhs.y; vec.z = rhs.z; };

		copy(vertex.position, mesh->mVertices[i]);
		copy(vertex.normal, mesh->mNormals[i]);
		if(model_type == ModelType::obj)
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

	auto load_maps = [this, &images, material](aiTextureType type, TexType textype)
	{
		auto maps = load_images(material, type, textype);
		images.insert(images.end(), maps.begin(), maps.end());
	};

	load_maps(aiTextureType_AMBIENT,  TexType::ambient);
	load_maps(aiTextureType_DIFFUSE,  TexType::diffuse);
	load_maps(aiTextureType_SPECULAR, TexType::specular);
	load_maps(aiTextureType_HEIGHT,   TexType::height);
	load_maps(aiTextureType_AMBIENT,  TexType::ambient);

	return AssimpMesh(vertices, indices, images);
}

std::vector<std::pair<Image, TexType>> 
AssimpModel::load_images(aiMaterial* mat, aiTextureType type, TexType textype)
{
	std::vector<std::pair<Image, TexType>> images;

	for (uint32_t i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString ainame;
		mat->GetTexture(type, i, &ainame);
		std::string name{ ainame.C_Str() };

		if(!name.empty())
		{ 
			if (images_cache.find(name) == images_cache.end())
			{
				std::cout << '\n' << "load image: " << (path + name);
				Image img(path + name, api);
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