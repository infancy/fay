#include "buffer.h"

namespace fay
{

Buffer::Buffer(std::vector<float>& positions, std::vector<uint32_t>& indices) : indices_size(indices.size())
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);	// 要求传入的顶点总是三维的

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Buffer::~Buffer()
{
	/*
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	*/
}

void Buffer::draw()
{
	//shader.enable();

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

}	//namespace fay