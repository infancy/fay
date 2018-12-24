#include <fstream>

#include"shader.h"

namespace fay
{

shader::shader(const char* glslPath)
{
	/*
	ifstream fp;
	fp.open(filename.c_str(), ios_base::in);
	if(fp) {
	string line, source;
	while(getline(fp, line)) {
	source.append(line);
	source.append("\r\n");
	}
	//copy to source
	LoadFromString(whichShader, source);
	}
	else
	cerr<<"Error loading shader: "<<filename<<endl;
	*/
}

shader::shader(const char* vertexPath, const char* fragmentPath, 
	const char* geometryPath)
{
	// retrieve the vertex/fragment source code from filePath
	std::string vertexSource, fragmentSource, geometrySource;
	std::ifstream vShaderFile, fShaderFile, gShaderFile;

	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexSource = vShaderStream.str();
		fragmentSource = fShaderStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (geometryPath != nullptr)
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometrySource = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	if (geometryPath != nullptr)
		create_shader(vertexSource.c_str(), fragmentSource.c_str(), geometrySource.c_str());
	else
		create_shader(vertexSource.c_str(), fragmentSource.c_str());
}

shader::shader(std::string& vertexSource, std::string& fragmentsource, std::string geometryPath)
{
	if (geometryPath != "")
		create_shader(vertexSource.c_str(), fragmentsource.c_str(), geometryPath.c_str());
	else
		create_shader(vertexSource.c_str(), fragmentsource.c_str());
}

void shader::create_shader(const char* vertexSouce, const char* fragmentSource,
	const char* geometrySource)
{
	GLuint vertex, fragment, geometry;

	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSouce, NULL);
	glCompileShader(vertex);
	check_compile_errors(vertex, "VERTEX");

	// fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);
	check_compile_errors(fragment, "FRAGMENT");

	// geometry shader
	if (geometrySource != nullptr)
	{
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geometrySource, NULL);
		glCompileShader(geometry);
		check_compile_errors(geometry, "GEOMETRY");
	}

	// shader Program
	program_id = glCreateProgram();

	glAttachShader(program_id, vertex);
	glAttachShader(program_id, fragment);
	if (geometrySource != nullptr)
		glAttachShader(program_id, geometry);

	glLinkProgram(program_id);
	check_compile_errors(program_id, "PROGRAM");

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometrySource != nullptr)
		glDeleteShader(geometry);
}

void shader::check_compile_errors(GLuint id, std::string type)
{
	GLint state;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &state);
		if (state == GL_FALSE)
		{
			glGetShaderInfoLog(id, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
				<< infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(id, GL_LINK_STATUS, &state);
		if (state == GL_FALSE)
		{
			glGetProgramInfoLog(id, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
				<< infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

} // namespace fay