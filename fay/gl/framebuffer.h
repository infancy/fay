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
	Framebuffer(uint32_t width, uint32_t height, bool multisample = false, GLenum format = GL_RGB)
	{
		glGenFramebuffers(1, &fbo);
		reset(width, height, multisample, format);
	}

	void reset(uint32_t width, uint32_t height, bool multisample = false, GLenum format = GL_RGB)
	{
		w = width, h = height;
		this->multisample = multisample;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLenum target = multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		tex_ = std::move(BaseTexture(target));
		tex_.set_format(format);
		// 外部传入图像的格式为 GL_UNSIGNED_BYTE
		if (multisample)
		{
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, format, width, height, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex_.id(), 0);	
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_.id(), 0);
		}

		// create a renderbuffer object for depth and stencil attachment (only write)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		// use a single renderbuffer object for both a depth AND stencil buffer.
		if (multisample)
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
		else
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "\nERROR::FRAMEBUFFER:: Framebuffer is not complete!";
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 创建一个用于保存普通纹理的 fb
		if (multisample)
		{
			glGenFramebuffers(1, &infbo);
			glBindFramebuffer(GL_FRAMEBUFFER, infbo);

			intex_ = std::move(BaseTexture(GL_TEXTURE_2D));
			intex_.set_format(format);

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intex_.id(), 0);
		
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "\nERROR::FRAMEBUFFER:: Framebuffer is not complete!";
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	uint32_t id() { return fbo; }

	// http://dict.youdao.com/w/eng/blit/#keyfrom=dict2.index
	// block transfer的缩写，本来应该缩写为blt，由于缩写中没有元音，无法读出来，所以添加了元音字母i使其成为了现在的blit。
	
	// 在绑定默认 fb 前调用
	void blit()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, infbo);
		glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	BaseTexture tex() 
	{ 
		return multisample ? intex_ : tex_;
	}

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
	bool multisample;
	int w, h;
	BaseTexture tex_, intex_;
	uint32_t fbo, rbo, infbo;
};

} // namespace fay

#endif // FAY_GL_FRAMEBUFFER_H
