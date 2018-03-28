#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_RESOURCES_H
#define FAY_GL_RESOURCES_H

#include "fay/core/fay.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace fay
{

struct vertex1
{
	glm::vec3 position;

	vertex1(float x = 0.f, float y = 0.f, float z = 0.f) : position{x, y, z} {}
};

struct vertex3
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;

	vertex3(
		glm::vec3 p = glm::vec3(0.f),
		glm::vec3 n = glm::vec3(0.f),
		glm::vec2 t = glm::vec2(0.f))
		: position{ p }, normal{ n }, texcoord{ t } {}
};

struct vertex5
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	vertex5(
		glm::vec3 p = glm::vec3(0.f),
		glm::vec3 n = glm::vec3(0.f),
		glm::vec2 t = glm::vec2(0.f),
		glm::vec3 a = glm::vec3(0.f),
		glm::vec3 b = glm::vec3(0.f))
		: position{ p }, normal{ n }, texcoord{ t }, tangent{ a }, bitangent{ b } {}
};

struct vertexN
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;

	union
	{
		float va[16];
	};
};

} // namespace fay

#endif // FAY_GL_RESOURCES_H
