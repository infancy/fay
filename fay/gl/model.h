#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_MODEL_H
#define FAY_GL_MODEL_H

#include "fay/gl/mesh.h"

namespace fay
{

class model
{
public:
	model(const std::string& filepath, model_type model_type = model_type::obj, bool gamma = false);

	// render the mesh, you need to bind texture by youself
	void draw(GLsizei sz = 1);
	// bind texture and render the mesh
	void draw(shader shader, GLsizei sz = 1);

public:
	// std::vector<mesh<vertex>> meshes;
	std::vector<mesh<vertex5>> meshes;
	bool gamma_correction;
};

class objmodel
{
public:
	objmodel(const std::string& filepath, bool gamma = false);

	void draw(GLsizei sz = 1);
	void draw(shader shader, GLsizei sz = 1);

public:
	// std::vector<mesh<vertex>> meshes;
	std::vector<mesh<vertex3>> meshes;
	bool gamma_correction;
};

} // namespace fay

#endif // FAY_GL_MODEL_H
