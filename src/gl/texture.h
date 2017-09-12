#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_GL_TEXTURE_H
#define FAY_GL_TEXTURE_H

#include <glad/glad.h>
#include "fay.h"

namespace fay
{

class Texture
{
public:
	Texture(const char* filename);

	GLuint id() const { return texture_id ; }

	void enable()  { glBindTexture(GL_TEXTURE_2D, texture_id); };
	void disable() { glBindTexture(GL_TEXTURE_2D, 0); };

private:
	GLuint texture_id;
};

}	//namespace fay


#endif //FAY_GL_TEXTURE_H
