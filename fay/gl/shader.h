#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_SHADER_H
#define FAY_GL_SHADER_H

#include "fay/utility/fay.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "fay/gl/texture.h"

namespace fay
{

class Shader // : public boost::noncopyable
{
public:
	Shader(const char* glslPath);
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	Shader(std::string& vertexSource, std::string& fragmentsource, std::string geometryPath = {});
	//~Shader(){ glDeleteProgram(program_id); }

	void enable()  { glUseProgram(program_id); }
	void disable() { glUseProgram(0); }

	//void delete_shader() { glDeleteProgram(program_id); }
	//void operator()(string& name, T value); vs shader[name] = value

	// utility uniform functions
	void set_bool(const std::string& name, bool value) const   { glUniform1i(glGetUniformLocation(program_id, name.c_str()), (int)value); }
	void set_int(const std::string& name, int value) const	   { glUniform1i(glGetUniformLocation(program_id, name.c_str()), value); }
	void set_float(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(program_id, name.c_str()), value); }

	void set_vec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]); }
	void set_vec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]); }
	void set_vec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, &value[0]); }
	
	void set_vec2(const std::string& name, float x, float y) const					 { glUniform2f(glGetUniformLocation(program_id, name.c_str()), x, y); }
	void set_vec3(const std::string& name, float x, float y, float z) const			 { glUniform3f(glGetUniformLocation(program_id, name.c_str()), x, y, z); }
	void set_vec4(const std::string& name, float x, float y, float z, float w) const { glUniform4f(glGetUniformLocation(program_id, name.c_str()), x, y, z, w); }

	void set_mat2(const std::string& name, const glm::mat2& mat) const { glUniformMatrix2fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
	void set_mat3(const std::string& name, const glm::mat3& mat) const { glUniformMatrix3fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
	
	void set_mat4(const std::string& name, const glm::mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
	
	// 使用 glUniform1i 给纹理采样器分配一个位置值
	void bind_texture(const std::string& sampler, int tex_unit, const BaseTexture& tex) const {
		glActiveTexture(GL_TEXTURE0 + tex_unit);	    // 激活第i号纹理单元
		glUniform1i(glGetUniformLocation(program_id, sampler.c_str()), tex_unit);	// 将第i号纹理单元连接到着色器中的sampler变量
		glBindTexture(tex.target(), tex.id()); } 	// 将纹理对象绑定到当前激活的纹理单元上

private:
	void create_shader(const char* vertexSouce, const char* fragmentSource, 
		const char* geometrySource = nullptr);

	void check_compile_errors(GLuint shader, std::string type);

public:
	unsigned int program_id {};
};

} // namespace fay

#endif // FAY_GL_SHADER_H

