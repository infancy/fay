#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_GL_BUFFER_H
#define FAY_GL_BUFFER_H

#include <glad/glad.h>
#include "fay.h"

namespace fay
{

class Buffer // : boost::noncopyable
{
public:
	//// 要求传入的顶点总是三维的
	Buffer(std::vector<float>& positions, std::vector<uint32_t>& indices);
	~Buffer();

	void draw();

private:
	GLuint indices_size;
	GLuint VBO, EBO, VAO;
};

}	//namespace fay


#endif //FAY_GL_BUFFER_H