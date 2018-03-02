#include "model.h"
#include "fay/utility/load_model.h"

namespace fay
{

Model::Model(const std::string& filepath, bool gamma) : gamma_correction(gamma)
{
	AssimpModel model(filepath);
	for (auto& mesh : model.meshes)
		this->meshes.emplace_back(mesh.vertices, mesh.indices, mesh.images);
}

/*
~modelMesh::modelMesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
*/

void Model::draw(Shader shader)
{
	for (auto& mesh : meshes)
		mesh.draw(shader);
}


}	//namespace fay
