#include "model.h"
#include "fay/utility/load_model.h"

namespace fay
{

Model::Model(const std::string& filepath, ModelType model_type, bool gamma) : gamma_correction(gamma)
{
	AssimpModel model(filepath, Thirdparty::gl, model_type);
	for (auto& mesh : model.meshes)
	{
	#ifdef _DEBUG
		std::cout << '\n'
			<< mesh.vertices.size()
			<< ' ' << mesh.indices.size()
			<< ' ' << mesh.images.size();
			// << ' ' << mesh.images[0].first.file_path() << '\n';
		if (mesh.images.size() != 0)
			std::cout << ' ' << mesh.images[0].first.file_path();
	#endif // _DEBUG

		this->meshes.emplace_back(mesh.vertices, mesh.indices, mesh.images);
	}
}

/*
~modelMesh::modelMesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
*/

void Model::draw()
{
	for (auto& mesh : meshes)
		mesh.draw();
}

void Model::draw(Shader shader)
{
	for (auto& mesh : meshes)
		mesh.draw(shader);
}

// -----------------------------------------------------------------------------

obj_Model::obj_Model(const std::string& filepath, bool gamma) : gamma_correction(gamma)
{
	ObjModel model(filepath);
	for (auto& mesh : model.meshes)
	{ 
		auto& objmesh = mesh.first;
		auto& objmat = mesh.second;

	#ifdef _DEBUG
		std::cout
			<< objmesh.name
			<< ' ' << objmesh.mat_name
			<< ' ' << objmat.name
			<< ' ' << objmat.map_Kd << '\n';
	#endif // _DEBUG

		std::vector<std::pair<Image, TexType>> images;

		if (!objmat.map_Ka.empty()) images.emplace_back( 
			Image(model.path + objmat.map_Ka, Thirdparty::gl), TexType::ambient);

		if (!objmat.map_Kd.empty()) images.emplace_back(
			Image(model.path + objmat.map_Kd, Thirdparty::gl), TexType::diffuse);

		if (!objmat.map_Ks.empty()) images.emplace_back(
			Image(model.path + objmat.map_Ks, Thirdparty::gl), TexType::specular);

		if (!objmat.map_Ke.empty()) images.emplace_back(
			Image(model.path + objmat.map_Ke, Thirdparty::gl), TexType::emissive);

		if (!objmat.map_d.empty()) images.emplace_back(
			Image(model.path + objmat.map_d, Thirdparty::gl), TexType::alpha);

		if (!objmat.map_bump.empty()) images.emplace_back(
			Image(model.path + objmat.map_bump, Thirdparty::gl), TexType::displace);

		this->meshes.emplace_back(objmesh.vertices, objmesh.indices, images);
	}
}

void obj_Model::draw()
{
	for (auto& mesh : meshes)
		mesh.draw();
}

void obj_Model::draw(Shader shader)
{
	for (auto& mesh : meshes)
		mesh.draw(shader);
}

}	//namespace fay
