#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_UNIFORM_H
#define FAY_GL_UNIFORM_H

#include "fay/gl/gl.h"

namespace fay
{

class unifrom
{
public:
	unifrom(uint32_t bytes) : bytes{ bytes }
	{
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, bytes, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	// templatetypename T>
	bool set(uint32_t offset, uint32_t size, void* data)
	{
		DCHECK((offset + size) <= bytes) << "uniform: out of range";

		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		// glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(T), data);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	uint32_t id() const { return ubo; }

private:
	uint32_t ubo;
	uint32_t bytes;
};


} // namespace fay

#endif // FAY_GL_UNIFORM_H
