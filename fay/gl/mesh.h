#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_MESH_H
#define FAY_GL_MESH_H

#include <assimp/Importer.hpp>

#include "fay/gl/buffer.h"
#include "fay/gl/shader.h"
#include "fay/gl/texture.h"
#include "fay/utility/image.h"

namespace fay
{

template<typename Vertex>
class Mesh 
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		std::vector<std::string> image_paths) : buffer(vertices, indices)
	{
		for (auto& filepath : image_paths)
			textures.emplace_back(filepath);
	}

	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		std::vector<std::pair<ImagePtr, TexType>> images) : buffer(vertices, indices)
	{
		for (auto& img : images)
			textures.emplace_back(img.first, img.second);
	}

	// 这样设置 Texture ？？
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		std::vector<Texture2D> textures) : buffer(vertices, indices)
	{
		textures = textures;
	}

	void bind_texture(Shader shader)
	{
		/*
		int count[8]{};
		std::string sampler;

		for (size_t i = 0; i < textures.size(); ++i)
		{
			size_t index = 0;
			for (; index < type.size(); ++index)
				if (textures[i].type() == type[index])
					break;
			LOG_IF(ERROR, index >= type.size()) << "TexType failed to choose: " << i;

			sampler = name[index];
			if (count[index]++ > 0)
				sampler += std::to_string(count[index]);
			// e.g. diff, diff1, diff2...
			shader.bind_texture(sampler, i, textures[i]);
		}
		*/
		for (size_t i = 0; i < textures.size(); ++i)
		{
			auto& tex = textures[i];

			LOG_IF(ERROR, type_name.find(tex.type()) == type_name.end())
				<< "TexType failed to choose: " << i;

			shader.bind_texture(type_name.at(tex.type()), i, tex);
		}
	}

	// render the mesh, you need to bind texture by youself
	void draw(GLsizei sz = 1)
	{
		buffer.draw(sz);
	}

	// bind texture and render the mesh
	void draw(Shader shader, GLsizei sz = 1)
	{
		// 还需要在 draw 前绑定其它数据
		// shader.enable();

		if (!textures.empty())
			bind_texture(shader);

		buffer.draw(sz);
		// shader.disable();
		glActiveTexture(GL_TEXTURE0);
	}

public:
	Buffer buffer;
	std::vector<Texture2D> textures;
	// Texture2DArray textures;

private:
	const std::unordered_map<TexType, std::string> type_name
	{
		{ TexType::alpha,    "Alpha"    },
		{ TexType::ambient,  "Ambient"  },
		{ TexType::diffuse,  "Diffuse"  },
		{ TexType::specular, "Specular" },
		{ TexType::parallax, "Parallax" },
		{ TexType::emissive, "Emissive" }
	};

};

} // namespace fay


#endif // FAY_GL_MESH_H