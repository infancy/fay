#include "buffer.h"

namespace fay
{
/*
template<typename Vertex>
buffer::buffer(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
	std::function<void()> set_vertex_attribPointer)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &ibo_);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	set_vertex_attribPointer();

	glBindVertexArray(0);
}
*/
// -----------------------------------------------------------------------------

buffer::buffer(std::vector<vertex1>& vertices, std::vector<uint32_t>& indices)
	: isz{ static_cast<GLsizei>(indices.size()) }
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &ibo_);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex1), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex1), (void*)0);

	glBindVertexArray(0);
}

buffer::buffer(std::vector<vertex3>& vertices, std::vector<uint32_t>& indices)
	: isz{ static_cast<GLsizei>(indices.size()) }
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &ibo_);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex3), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex3), (void*)offsetof(vertex3, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex3), (void*)offsetof(vertex3, texcoord));

	glBindVertexArray(0);
}

buffer::buffer(std::vector<vertex5>& vertices, std::vector<uint32_t>& indices)
	: isz{ static_cast<GLsizei>(indices.size()) }
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo_);
	glGenBuffers(1, &ibo_);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex5), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	// set the vertex attribute pointers
	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex5), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex5), (void*)offsetof(vertex5, normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex5), (void*)offsetof(vertex5, texcoord));
	// vertex tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex5), (void*)offsetof(vertex5, tangent));
	// vertex bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex5), (void*)offsetof(vertex5, bitangent));

	glBindVertexArray(0);
}

// -----------------------------------------------------------------------------

void buffer::draw(GLsizei sz)
{
	DCHECK(sz >= 1) << "buffer: error size";
	glBindVertexArray(vao);
	if(sz == 1)
		glDrawElements(GL_TRIANGLES, isz, GL_UNSIGNED_INT, 0);
	else
		glDrawElementsInstanced(GL_TRIANGLES, isz, GL_UNSIGNED_INT, 0, sz);
	glBindVertexArray(0);
}

} // namespace fay