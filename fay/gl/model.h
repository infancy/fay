#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_MODEL_H
#define FAY_GL_MODEL_H

#include "fay/gl/mesh.h"

namespace fay
{

class Model
{
public:
	Model(const std::string& filepath, ModelType model_type = ModelType::obj, bool gamma = false);

	// render the mesh, you need to bind texture by youself
	void draw();
	// bind texture and render the mesh
	void draw(Shader shader);

public:
	// std::vector<Mesh<Vertex>> meshes;
	std::vector<Mesh<Vertex5>> meshes;
	bool gamma_correction;
};

class obj_Model
{
public:
	obj_Model(const std::string& filepath, bool gamma = false);

	void draw();
	void draw(Shader shader);

public:
	// std::vector<Mesh<Vertex>> meshes;
	std::vector<Mesh<Vertex3>> meshes;
	bool gamma_correction;
};

} // namespace fay

#endif // FAY_GL_MODEL_H
