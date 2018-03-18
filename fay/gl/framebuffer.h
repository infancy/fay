#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_FRAMEBUFFER_H
#define FAY_GL_FRAMEBUFFER_H

#include "fay/gl/gl.h"

namespace fay
{

class Framebuffer
{
public:
	Framebuffer(uint32_t width, uint32_t height, GLenum glformat = GL_RGB)
	{
		glGenFramebuffers(1, &fbo);
		reset(width, height, glformat);
	}

	void reset(uint32_t width, uint32_t height, GLenum glformat)
	{
		w = width, h = height; fmt = glformat;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		//glViewport(0, 0, width, height);

		//	glDeleteBuffers(1, &tbo);
		//	glDeleteBuffers(1, &rbo);

		// create a color attachment texture
		glGenTextures(1, &tbo);
		glBindTexture(GL_TEXTURE_2D, tbo);
		// 外部传入图像的格式为 GL_UNSIGNED_BYTE
		glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, glformat, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tbo, 0);

		// create a renderbuffer object for depth and stencil attachment (only write)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		// now actually attach it
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "\nERROR::FRAMEBUFFER:: Framebuffer is not complete!";

		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	uint32_t fbo_id() { return fbo; }

	uint32_t tex_id() { return tbo; }

	int width()  const { return w; }
	int height() const { return h; }
	GLenum format() const { return fmt; }

	void enable(glm::vec3 clear_color) 
	{ 
		gl_enable_framebuffer(fbo, w, h, clear_color);
	}

	void disable() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

private:
	//bool is_reset{ false };
	int w{}, h{};
	GLenum fmt;
	uint32_t fbo{}, tbo{}, rbo{};
};

} // namespace fay

#endif // FAY_GL_FRAMEBUFFER_H
