#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_FRAMEBUFFER_H
#define FAY_GL_FRAMEBUFFER_H

#include "fay/gl/gl.h"
#include "fay/gl/texture.h"

namespace fay
{

class base_framebuffer
{
public:
	base_framebuffer()
	{
		glGenFramebuffers(1, &fbo);
		// 不要在构造函数里调用虚函数
		// reset(width, height, format, type);
	}

	virtual void reset(uint32_t width, uint32_t height, GLenum format, GLenum type) = 0;

	uint32_t id() { return fbo; }
	virtual base_texture tex() { return tex_; }

	int width()  const { return w; }
	int height() const { return h; }
	GLenum format() const { return tex_.format(); }

	void enable(glm::vec3 clear_color)
	{
		gl_enable_framebuffer(fbo, w, h, clear_color);
	}

	void disable() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

protected:
	void set_and_enable(uint32_t width, uint32_t height)
	{
		w = width, h = height;

		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	}

	void check_and_disable()
	{
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "\nERROR::FRAMEBUFFER:: Framebuffer is not complete!";
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

protected:
	//bool is_reset{ false };
	int w, h;
	base_texture tex_;
	uint32_t fbo, rbo;
};

// -----------------------------------------------------------------------------

class framebuffer : public base_framebuffer
{
public:
	framebuffer(uint32_t width, uint32_t height, 
		GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE)
	{
		reset(width, height, format, type);
	}

	virtual void reset(uint32_t width, uint32_t height, GLenum format, GLenum type)
	{
		set_and_enable(width, height);

		DLOG(INFO) << "framebuffer: reset";
		tex_ = std::move(base_texture(GL_TEXTURE_2D));
		tex_.set_format(format);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_.id(), 0);

		// create a renderbuffer object for depth and stencil attachment (only write)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		
		check_and_disable();
	}
};

// -----------------------------------------------------------------------------

class multisample_framebuffer : public base_framebuffer
{
public:
	multisample_framebuffer(uint32_t width, uint32_t height, 
		GLenum format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE)
	{
		reset(width, height, format, type);
	}

	virtual void reset(uint32_t width, uint32_t height, GLenum format, GLenum type) override
	{
		set_and_enable(width, height);

		DLOG(INFO) << "multisample_framebuffer: reset";
		tex_ = std::move(base_texture(GL_TEXTURE_2D_MULTISAMPLE));
		tex_.set_format(format);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, format, width, height, GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex_.id(), 0);

		// create a renderbuffer object for depth and stencil attachment (only write)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		check_and_disable();

		// 创建一个用于保存普通纹理的 fb
		glGenFramebuffers(1, &infbo);
		glBindFramebuffer(GL_FRAMEBUFFER, infbo);

		intex_ = std::move(base_texture(GL_TEXTURE_2D));
		intex_.set_format(format);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intex_.id(), 0);

		check_and_disable();
	}

	// http://dict.youdao.com/w/eng/blit/#keyfrom=dict2.index
	// block transfer的缩写，本来应该缩写为blt，由于缩写中没有元音，无法读出来，所以添加了元音字母i使其成为了现在的blit。

	// 在绑定默认 fb 前调用
	void blit()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, infbo);
		glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	virtual base_texture tex() override
	{
		return intex_;
	}

private:
	base_texture intex_;
	uint32_t infbo;
};

// -----------------------------------------------------------------------------

class shadowmap_framebuffer : public base_framebuffer
{
public:
	shadowmap_framebuffer(uint32_t width = 1024, uint32_t height = 1024,
		GLenum format = GL_DEPTH_COMPONENT, GLenum type = GL_FLOAT)
	{
		reset(width, height, format, type);
	}

	virtual void reset(uint32_t width, uint32_t height, GLenum format, GLenum type) override
	{
		set_and_enable(width, height);

		DLOG(INFO) << "shadowmap_framebuffer: reset";
		tex_ = std::move(base_texture(GL_TEXTURE_2D, GL_NEAREST, GL_CLAMP_TO_BORDER));
		tex_.set_border_color({ 1.f, 1.f, 1.f, 1.f });
		tex_.set_format(format);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_.id(), 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		check_and_disable();
	}
};

// -----------------------------------------------------------------------------

class gbuffer_framebuffer : public base_framebuffer
{
public:
	gbuffer_framebuffer(uint32_t width, uint32_t height,
		GLenum format = GL_NEAREST, GLenum type = GL_FLOAT)
	{
		reset(width, height, format, type);
	}

	virtual void reset(uint32_t width, uint32_t height, GLenum format, GLenum type) override
	{
		set_and_enable(width, height);

		texs_.resize(3);
		// position color buffer
		texs_[0] = texture2d(GL_NEAREST, GL_REPEAT, texture_type::diffuse);
		texs_[0].create(GL_RGB16F, w, h, GL_RGB, GL_FLOAT, NULL, false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texs_[0].id(), 0);
		// normal color buffer
		texs_[1] = texture2d(GL_NEAREST, GL_REPEAT, texture_type::diffuse);
		texs_[1].create(GL_RGB16F, w, h, GL_RGB, GL_FLOAT, NULL, false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texs_[1].id(), 0);
		// color + specular color buffer
		texs_[2] = texture2d(GL_NEAREST, GL_REPEAT, texture_type::diffuse);
		texs_[2].create(GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texs_[2].id(), 0);
			
		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
		// create and attach depth buffer (renderbuffer)
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

		check_and_disable();
	}

	const std::vector<texture2d>& texs() { return texs_; }

private:
	std::vector<texture2d> texs_;
};

} // namespace fay

#endif // FAY_GL_FRAMEBUFFER_H
