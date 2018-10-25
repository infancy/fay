#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_MESH_H
#define FAY_GL_MESH_H

#include "fay/gl/buffer.h"
#include "fay/gl/shader.h"
#include "fay/gl/texture.h"

namespace fay
{

template<typename vertex>
class mesh 
{
public:
	mesh(std::vector<vertex> vertices, std::vector<uint32_t> indices,
		std::vector<std::string> image_paths) : buf(vertices, indices)
	{
		for (auto& filepath : image_paths)
			textures.emplace_back(filepath);
	}

	mesh(std::vector<vertex> vertices, std::vector<uint32_t> indices,
		std::vector<std::pair<image_ptr, texture_format>> images) : buf(vertices, indices)
	{
		for (auto& img : images)
			textures.emplace_back(img.first, img.second);
	}

	// 这样设置 Texture ？？
	mesh(std::vector<vertex> vertices, std::vector<uint32_t> indices,
		std::vector<texture2d> textures) : buf(vertices, indices)
	{
		textures = textures;
	}

	void bind_texture(shader shader)
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
			LOG_IF(ERROR, index >= type.size()) << "texture_format failed to choose: " << i;

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
				<< "texture_format failed to choose: " << i;

			shader.bind_texture(type_name.at(tex.type()), i, tex);
		}
	}

	// render the mesh, you need to bind texture by youself
	void draw(GLsizei sz = 1)
	{
		buf.draw(sz);
	}

	// bind texture and render the mesh
	void draw(shader shader, GLsizei sz = 1)
	{
		// 还需要在 draw 前绑定其它数据
		// shader.enable();

		if (!textures.empty())
			bind_texture(shader);

		buf.draw(sz);
		// shader.disable();
		glActiveTexture(GL_TEXTURE0);
	}

public:
	buffer buf;
	std::vector<texture2d> textures;
	// texture2d_array textures;

private:
	const std::unordered_map<texture_format, std::string> type_name
	{
		{ texture_format::alpha,    "Alpha"    },
		{ texture_format::ambient,  "Ambient"  },
		{ texture_format::diffuse,  "Diffuse"  },
		{ texture_format::specular, "Specular" },
		{ texture_format::parallax, "Parallax" },
		{ texture_format::emissive, "Emissive" }
	};

};

} // namespace fay


#endif // FAY_GL_MESH_H