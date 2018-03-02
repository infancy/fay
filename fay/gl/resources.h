#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_RESOURCES_H
#define FAY_GL_RESOURCES_H

#include "fay/utility/fay.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace fay
{

struct Vertex1
{
	glm::vec3 position;

	Vertex1(float x, float y, float z) : position{x, y, z} {}
};

struct Vertex3
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct Vertex5
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

	glm::vec3 tangent;
	glm::vec3 bitangent;
};

} // namespace fay

#endif // FAY_GL_RESOURCES_H
