#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_OPENGL_MESH_H
#define FAY_OPENGL_MESH_H

#include <assimp/Importer.hpp>
//#include <glad/glad.h>
#include "shader.h"

namespace fay
{

struct Vertex 
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct Texture	// Texture 只是一个 handle
{
	Texture(GLuint id, std::string type, aiString path) : id(id), type(type), filepath(path) {}

	GLuint	    id;		
	std::string type;	// enum class Type { diffuse, specular, normal, height } type;
	aiString    filepath;
};

class Mesh 
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures);
	~Mesh();

	// render the mesh
	void draw(Shader shader);

private:
	// initializes all the buffer objects/arrays
	void setup_mesh();

public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;
	GLuint VAO;

private:
	GLuint VBO, EBO;
};

}	//namespace fay


#endif //FAY_OPENGL_MESH_H