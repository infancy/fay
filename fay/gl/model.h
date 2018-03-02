#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_OPENGL_MODEL_H
#define FAY_OPENGL_MODEL_H

#include "fay/gl/mesh.h"

namespace fay
{

class Model
{
public:
	Model(const std::string& filepath, bool gamma = false);

	void draw(Shader shader);

public:
	// std::vector<Mesh<Vertex>> meshes;
	std::vector<Mesh<Vertex5>> meshes;
	bool gamma_correction;
};

} // namespace fay

#endif // FAY_OPENGL_MODEL_H
