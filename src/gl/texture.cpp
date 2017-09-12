#include "texture.h"
#include "stb_image.h"

namespace fay
{

Texture::Texture(const char* filename)
{
	std::string filepath = filename;

	glGenTextures(1, &texture_id);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		LOG(INFO) << "Texture successed to load at path: " << filename;
		GLenum format{};

		switch (nrComponents)
		{
		case 1: format = GL_RED; break;
		case 3: format = GL_RGB; break;
		case 4: format = GL_RGBA; break;
		default:
			LOG(ERROR) << "nrComponents failed to choose"; break;
		}

		glBindTexture(GL_TEXTURE_2D, texture_id);	//之后任何的纹理指令都作用于当前绑定的纹理
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
		LOG(ERROR) << "Texture failed to load at path: " << filename;

	stbi_image_free(data);
}

} //namespace fay