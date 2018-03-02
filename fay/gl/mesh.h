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

	// ’‚—˘…Ë÷√ Texture £ø£ø
	Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
		std::vector<Texture> textures) : buffer(vertices, indices)
	{
		textures = textures;
	}

	// render the mesh
	void draw()
	{
		buffer.draw();
	}

	// bind texture and render the mesh
	void draw(Shader shader)
	{
		// constexpr int a = '0' - '/';
		// std::string  diff{ "diff/" }, spec{ "spec/" }, norm{ "norm/" }, heig{ "heig/" };
		std::string  diff{ "diff" }, spec{ "spec" }, norm{ "normal" }, heig{ "height" };
		int d{}, s{}, n{}, h{};
		std::string sampler;

		for (size_t i = 0; i < textures.size(); ++i)
		{
			switch (textures[i].type())
			{
			case TexType::diffuse:  sampler = diff; if (d++ > 0) sampler += std::to_string(d); break;
			case TexType::specular: sampler = spec; if (s++ > 0) sampler += std::to_string(s); break;
			case TexType::normals:  sampler = norm; if (n++ > 0) sampler += std::to_string(n); break;
			case TexType::height:   sampler = heig; if (h++ > 0) sampler += std::to_string(h); break;
			default:
				LOG(ERROR) << "TexType failed to choose: " << i; break;
			}
			// e.g. diff, diff1, diff2...
			shader.bind_texture(sampler, i, textures[i].id());
		}

		buffer.draw();

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

public:
	Buffer buffer;
	std::vector<Texture2D> textures;
	// Texture2DArray textures;
};

} // namespace fay


#endif // FAY_GL_MESH_H