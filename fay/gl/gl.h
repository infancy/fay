#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_GL_H
#define FAY_GL_GL_H

#include "fay/core/fay.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace fay
{

inline void gl_enable_framebuffer(uint32_t fbo, int w, int h, glm::vec3 clear_color)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, w, h);
	glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

inline void gl_check_errors() { CHECK(glGetError() == GL_NO_ERROR); }

} // namespace fay


#endif // FAY_GL_GL_H

