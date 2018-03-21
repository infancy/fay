#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_FRAMEBUFFER_H
#define FAY_GL_FRAMEBUFFER_H

#include "fay/gl/gl.h"
#include "fay/gl/texture.h"

namespace fay
{

class Framebuffer
{
public:
	Framebuffer(uint32_t width, uint32_t height, GLenum format = GL_RGB)
	{
		glGenFramebuffers(1, &fbo);
		reset(width, height, format);
	}

	void reset(uint32_t width, uint32_t height, GLenum format)
	{
		w = width, h = height;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// create a color attachment texture
		tex_ = std::move(BaseTexture(GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT));
		tex_.set_format(format);
		// 外部传入图像的格式为 GL_UNSIGNED_BYTE
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_.id(), 0);

		// create a renderbuffer object for depth and stencil attachment (only write)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		// now actually attach it
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "\nERROR::FRAMEBUFFER:: Framebuffer is not complete!";

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	uint32_t fbo_id() { return fbo; }

	BaseTexture tex() { return tex_; }

	int width()  const { return w; }
	int height() const { return h; }
	GLenum format() const { return tex_.format(); }

	void enable(glm::vec3 clear_color) 
	{ 
		gl_enable_framebuffer(fbo, w, h, clear_color);
	}

	void disable() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

private:
	//bool is_reset{ false };
	int w{}, h{};
	BaseTexture tex_;
	uint32_t fbo{}, rbo{};
};

} // namespace fay

#endif // FAY_GL_FRAMEBUFFER_H
