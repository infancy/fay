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

using vertex1 = glm::vec3;	// for single-member-variables struct, maybe it is a better way
/*

struct vertex1 
{ 
	glm::vec3 position{0.f}; 
	// complier generate: vertex1(glm::vec3 v) { ... }  
};

{
	glm::vec3 v0;
	glm::vec3 v1(0.f);
	glm::vec3 v2(0.f, 0.f, 0.f);
	glm::vec3 v3{ 0.f };
	glm::vec3 v4{ 0.f, 0.f, 0.f };
	//glm::vec3 v5 = 0.f;	// the constructor is explicit
	//glm::vec3 v6 = { 0.f };	// same
	glm::vec3 v7 = glm::vec3(0.f);
}
{
	vertex1 v0;
	//vertex1 v1(0.f);
	//vertex1 v2(0.f, 0.f, 0.f);
	//vertex1 v3{ 0.f };
	//vertex1 v4{ 0.f, 0.f, 0.f };
	//vertex1 v5 = 0.f;
	//vertex1 v6 = { 0.f };
	//vertex1 v7 = vertex1(0.f);

	//vertex1 v8({0.f});
	//vertex1 v9({0.f, 0.f, 0.f});
	vertex1 va{ {0.f, 0.f, 0.f} };
	//vertex1 vb = { {0.f} };	// the constructor is explicit
	//vertex1 vc = vertex1({0.f});
	//vertex1 vd = vertex1({ 0.f, 0.f, 0.f });
	//vertex1 ve = vertex1{{ 0.f }}; // can't convert init_list to vertex
	vertex1 vf = vertex1{{ 0.f, 0.f, 0.f }};
}
*/
struct vertex3
{
	glm::vec3 position{0.f};
	glm::vec3 normal  {0.f};
	glm::vec2 texcoord{0.f};
};

struct vertex5
{
	glm::vec3 position{0.f};
	glm::vec3 normal  {0.f};
	glm::vec2 texcoord{0.f};

	glm::vec3 tangent  {0.f};
	glm::vec3 bitangent{0.f};
};

struct vertexN
{
	glm::vec3 position{0.f};
	glm::vec3 normal  {0.f};
	glm::vec2 texcoord{0.f};

	union
	{
		float va[16]{};
	};
};

} // namespace fay

#endif // FAY_GL_RESOURCES_H
