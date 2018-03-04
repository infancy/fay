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
		std::vector<std::pair<Image, TexType>> images) : buffer(vertices, indices)
	{
		for (auto& img : images)
			textures.emplace_back(img.first, img.second, true);	// true
	}

	// 这样设置 Texture ？？
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		std::vector<Texture> textures) : buffer(vertices, indices)
	{
		textures = textures;
	}

	// render the mesh, you need to bind texture by youself
	void draw()
	{
		buffer.draw();
	}

	// bind texture and render the mesh
	void draw(Shader shader)
	{
		int count[8]{};
		std::string sampler;

		// 还需要在 draw 前绑定其它数据
		// shader.enable();

		for (size_t i = 0; i < textures.size(); ++i)
		{
			size_t sz = 0;
			for (; sz < type.size(); ++sz)
				if (textures[i].type() == type[sz])
					break;
			LOG_IF(ERROR, sz >= type.size()) << "TexType failed to choose: " << i;

			sampler = name[sz];
			if (count[sz]++ > 0) 
				sampler += std::to_string(count[sz]);
			// e.g. diff, diff1, diff2...
			shader.bind_texture(sampler, i, textures[i].id());
		}
		buffer.draw();
		// shader.disable();
		glActiveTexture(GL_TEXTURE0);
	}

public:
	Buffer buffer;
	std::vector<Texture2D> textures;
	// Texture2DArray textures;

private:
	// std::unordered_map<TexType, int> type_num; // hash<TexType>
	std::vector<TexType> type
	{ 
		TexType::diffuse, TexType::specular, TexType::ambient, TexType::emissive,
		TexType::alpha, TexType::normals, TexType::displace, TexType::height 
	};
	std::vector<std::string> name	// name in the shader
	{
		{ "diff" }, { "spec" }, { "ambient" }, { "emissive" }, 
		{ "alpha" }, { "normal" }, { "displace" }, { "height" }
	};

};

} // namespace fay


#endif // FAY_GL_MESH_H