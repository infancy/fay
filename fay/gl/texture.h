#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_TEXTURE_H
#define FAY_GL_TEXTURE_H

#include "gl.h"
#include "fay/utility/image.h"

namespace fay
{

class Texture
{
public: 
	// Texture(GLuint id) : texture_id{ id } {}
	// 只负责创建纹理，且不指定纹理单元，设置这样的接口以免被误用
	Texture(GLenum format = GL_TEXTURE_2D, GLint filtering = GL_LINEAR, GLint wrap = GL_REPEAT);

	GLuint id() const { return texture_id; }
	/*
	void enable(uint32_t tex_unit, Shader& shader, std::string sampler) {
		glActiveTexture(GL_TEXTURE0 + tex_unit);	
		shader.set_texture_unit(sampler, tex_unit);			
		glBindTexture(GL_TEXTURE_2D, texture_id);}
	void disable() {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);}
	*/
private:
	GLuint texture_id;
	//GLuint texture_unit;
};

// 创建纹理后，子类有着不同的加载资源方式

// -----------------------------------------------------------------------------

class Texture2D : public Texture
{
public:
	// Texture2D(GLuint id) : Texture(id) {}
	Texture2D(const std::string& filepath, TexType textype = TexType::diffuse, bool Mipmap = true);
	
	Texture2D(const Image& img, TexType textype = TexType::diffuse, bool Mipmap = true);
	
	Texture2D(GLint internalFormat, GLsizei width, GLsizei height,
		GLenum format, GLenum type, unsigned char* pixels, bool Mipmap = true, TexType textype = TexType::diffuse);
	/*     
	void enable(Shader& shader, std::string& sampler, uint32_t tex_unit) {
		glActiveTexture(GL_TEXTURE0 + tex_unit);	    // 激活第i号纹理单元
		shader.set_texture_unit(sampler, tex_unit);		// 将第i号纹理单元连接到着色器中的sampler变量	
		glBindTexture(GL_TEXTURE_2D, texture_id); } 	// 将纹理对象绑定到当前激活的纹理单元上
	void disable() { 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0); }
	*/

	int width()  const { return w; }
	int height() const { return h; }
	TexType type() const { return texture_type; }

private:
	void create_texture2d(GLint internalFormat, GLsizei width, GLsizei height,
		GLenum format, GLenum type, const unsigned char* pixels, bool Mipmap);

private:
	int w, h;
	TexType texture_type;
};

// -----------------------------------------------------------------------------

class Texture2DArray : public Texture
{
public:
	// 必须保证 GL_UNSIGNED_BYTE 的 type，且每张纹理的尺寸相同
	// TODO：以最大纹理的长宽来设置 glTexImage3D
	Texture2DArray(std::vector<std::string> material_names, std::string path = {});

	size_t size() const { return material_nums; }

private:
	size_t material_nums;
};

// 通过纹理来传输数据 ------------------------------------------------------------

// Texture1D??
template<typename T>
class TextureData : public Texture
{
public:
	// 必须考虑 width 和 format 的关系，如 data<float> 和 GL_RGBA，则 width = data.size() / 4
	TextureData(GLint internalFormat, GLsizei width, 
		GLenum format, GLenum type, std::vector<T>& data) :
		Texture(GL_TEXTURE_2D, GL_NEAREST, GL_REPEAT)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, 1, 0,
			format, type, data.data());
		gl_check_errors();
	}
};

//glm::vec4、glm::uvec4、glm::ivec4
template<typename T>
class TextureDataArray : public Texture
{
public:
	TextureDataArray(GLint internalFormat, GLenum format, GLenum type, std::vector<std::vector<T>>& datas) :
		Texture(GL_TEXTURE_2D_ARRAY, GL_NEAREST, GL_REPEAT)
	{
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
