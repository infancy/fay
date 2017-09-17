#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_GL_TEXTURE_H
#define FAY_GL_TEXTURE_H

#include "gl_utility.h"
#include "shader.h"

namespace fay
{

class Texture
{
public: 
	// 只负责创建纹理，且不指定纹理单元，这样的接口可以防止被误用
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

class Texture2D
{
public:
	Texture2D(const char* filename);

	GLuint id() const { return texture_id ; }
	/*
	void enable(uint32_t tex_unit = 0)  {
		glActiveTexture(GL_TEXTURE0 + tex_unit);	         
		glBindTexture(GL_TEXTURE_2D, texture_id); }      
	void enable(Shader& shader, std::string& sampler, uint32_t tex_unit) {
		glActiveTexture(GL_TEXTURE0 + tex_unit);	    // 激活第i号纹理单元
		shader.set_texture_unit(sampler, tex_unit);		// 将第i号纹理单元连接到着色器中的sampler变量	
		glBindTexture(GL_TEXTURE_2D, texture_id); } 	// 将纹理对象绑定到当前激活的纹理单元上
	void disable() { 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0); }
	*/
private:
	GLuint texture_id;
};

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

class TextureArray : public Texture
{
public:
	// 必须保证 GL_UNSIGNED_BYTE 的 type，且尺寸相同
	TextureArray(std::vector<std::string> material_names, std::string filepath = {});

	size_t size() const { return material_nums; }

private:
	size_t material_nums;
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

}	//namespace fay

#endif //FAY_GL_TEXTURE_H
