#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_TEXTURE_H
#define FAY_GL_TEXTURE_H

#include <array>

#include "fay/gl/gl.h"
#include "fay/utility/image.h"

namespace fay
{

// #if OPENGL_VERSION < ...

class base_texture
{
public: 
	// 只负责创建纹理，且不指定纹理单元，设置这样的接口以免被误用
	base_texture() {}
	base_texture(GLenum target, GLint filtering = GL_LINEAR, GLint wrap = GL_REPEAT);

	void set_format(GLenum format) { format_ = format; }
	void set_border_color(std::array<GLfloat, 4> borderColor);

	GLuint id() const { return id_; }
	GLenum target() const { return target_; }
	GLenum format() const { return format_; }

protected:
	GLuint id_;
	GLenum target_;
	GLenum format_;	// TODO: internalFormat??
};

// 创建纹理后，子类有着不同的加载资源方式

// -----------------------------------------------------------------------------

class texture2d : public base_texture
{
public:
	texture2d(const std::string& filepath, texture_type textype = texture_type::diffuse, bool Mipmap = true);
	
	texture2d(const image_ptr& img, texture_type textype = texture_type::diffuse, bool Mipmap = true);
	
	texture2d(GLint filtering = GL_LINEAR, GLint wrap = GL_REPEAT, texture_type textype = texture_type::diffuse);

	void create(GLint internalFormat, GLsizei width, GLsizei height,
		GLenum format, GLenum type, const uint8_t* data, bool Mipmap = true);

	int width()  const { return w; }
	int height() const { return h; }

	texture_type type() const { return textype_; }

private:
	int w, h;
	texture_type textype_;
};

// -----------------------------------------------------------------------------

class texture_cube : public base_texture
{
public:
	// filepath + 6 * filename( right, left, top, bottom, back, front )
	texture_cube(const std::string files[7], texture_type textype = texture_type::cubemap)
		: base_texture(GL_TEXTURE_CUBE_MAP, GL_LINEAR, GL_CLAMP_TO_EDGE), texture_type{ textype }
	{
		for (int i = 0; i < 6; ++i)
		{
			image_ptr img(files[0] + files[i + 1]/*, third_party::gl*/);

			if (i == 0)
				format_ = img.gl_format();

			CHECK(format_ == img.gl_format()) << "texture_cube: channels error!";
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format_,
				img.width(), img.height(), 0, format_, GL_UNSIGNED_BYTE, img.data());
		}
		gl_check_errors();
	}

	texture_type type() const { return texture_type; }

private:
	texture_type texture_type;
};

// -----------------------------------------------------------------------------

class texture2d_array : public base_texture
{
public:
	// 必须保证 GL_UNSIGNED_BYTE 的 type，且每张纹理的尺寸相同
	// TODO：以最大纹理的长宽来设置 glTexImage3D
	texture2d_array(std::vector<std::string> material_names, std::string path = {});

	size_t size() const { return material_nums; }

private:
	size_t material_nums;
};

// 通过纹理来传输数据 ------------------------------------------------------------

// Texture1D??
template<typename T>
class texture_data : public base_texture
{
public:
	// 必须考虑 width 和 format 的关系，如 data<float> 和 GL_RGBA，则 width = data.size() / 4
	texture_data(GLint internalFormat, GLsizei width, 
		GLenum format, GLenum type, std::vector<T>& data) :
		base_texture(GL_TEXTURE_2D, GL_NEAREST, GL_REPEAT)
	{
		format_ = internalFormat;
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, 1, 0,
			format, type, data.data());
		gl_check_errors();
	}
};

//glm::vec4、glm::uvec4、glm::ivec4
template<typename T>
class texture_data_array : public base_texture
{
public:
	texture_data_array(GLint internalFormat, GLenum format, GLenum type, std::vector<std::vector<T>>& datas) :
		base_texture(GL_TEXTURE_2D_ARRAY, GL_NEAREST, GL_REPEAT)
	{
		format_ = internalFormat;
		// 先创建一个 2D 纹理数组
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat,
			datas[0].size(), 1, datas.size(),     0,
			format, type, NULL);

		for (auto& data : datas)
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 
				0, 0, i, 
				data.size(), 1, 1, format, type, data.data());

		gl_check_errors();
	}
};

} // namespace fay

#endif // FAY_GL_TEXTURE_H
