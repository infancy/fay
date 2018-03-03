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

	Vertex1(float x = {}, float y = {}, float z = {}) : position{x, y, z} {}
};

struct Vertex3
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;

	Vertex3(
		glm::vec3 p = glm::vec3(),
		glm::vec3 n = glm::vec3(),
		glm::vec2 t = glm::vec2())
		: position{ p }, normal{ n }, texcoord{ t } {}
};

struct Vertex5
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;

	glm::vec3 tangent;
	glm::vec3 bitangent;

	Vertex5(
		glm::vec3 p = glm::vec3(), 
		glm::vec3 n = glm::vec3(),
		glm::vec2 t = glm::vec2(),
		glm::vec3 a = glm::vec3(),
		glm::vec3 b = glm::vec3()) 
		: position{ p }, normal{ n }, texcoord{ t }, tangent{ a }, bitangent{ b } {}
};

} // namespace fay

#endif // FAY_GL_RESOURCES_H
