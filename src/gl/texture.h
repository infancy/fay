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
		glActiveTexture(GL_TEXTURE0 + tex_unit);	    // 激活第i号纹理单元
		shader.set_texture_unit(sampler, tex_unit);		// 将第i号纹理单元连接到着色器中的sampler变量	
		glBindTexture(GL_TEXTURE_2D, texture_id);
	} 	// 将纹理对象绑定到当前激活的纹理单元上
	void disable() {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
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

class TextureArray : public Texture
{
public:
	TextureArray(std::vector<std::string> material_names, std::string filepath = {});

	size_t size() const { return material_nums; }

private:
	size_t material_nums;
};

// Texture1D??
template<typename T>
class TextureData : public Texture
{
public:
	TextureData(GLint internalFormat, GLsizei width, 
		GLenum format, GLenum type, std::vector<T>& data) :
		Texture(GL_TEXTURE_2D, GL_NEAREST, GL_REPEAT)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, 1, 0,
			format, type, data.data());
		gl_check_errors();
	}
};

}	//namespace fay

#endif //FAY_GL_TEXTURE_H
