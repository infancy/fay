#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_BUFFER_H
#define FAY_GL_BUFFER_H

#include "fay/gl/geometry.h"

namespace fay
{

// template<int VertexAttrib>
// template<bool onlyPosition> mesh
class buffer // : boost::noncopyable
{
public:
	// vec3 position, vec3 normal, vec2 texCoords, vec3 tangent, vec3 bitangent
	// vertexAttribPointer
	buffer(std::vector<vertex1>& vertices, std::vector<uint32_t>& indices);
	buffer(std::vector<vertex3>& vertices, std::vector<uint32_t>& indices);
	buffer(std::vector<vertex5>& vertices, std::vector<uint32_t>& indices);
	//buffer(std::vector<vertex5>& vertices, std::vector<uint32_t>& indices, std::function<void()> vertexattribpointer);

	uint32_t id() const { return vao; }
	uint32_t vbo() const { return vbo_; }
	uint32_t size() const { return isz; }

	void draw(GLsizei sz = 1);

	~buffer(){ /* glDeleteVertexArrays(1, &VAO); glDeleteBuffers(1, &VBO); glDeleteBuffers(1, &IBO); */ }

private:
	GLuint vao{}, vbo_{}, ibo_{};
	GLsizei isz{};	// indices_size
};

} // namespace fay

#endif // FAY_GL_BUFFER_H