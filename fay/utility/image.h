#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_IMAGE_H
#define FAY_UTILITY_IMAGE_H

#include "fay/gl/gl.h"
#include <stb_image.h>

namespace fay
{

class Image	// shared_image
{
public:
	Image(const std::string& filepath = {}, Thirdparty thirdparty = Thirdparty::none) :
		filepath{ filepath }, thirdparty{ thirdparty }
	{
		LOG_IF(ERROR, filepath.empty()) << "image path is empty: ";
		if (thirdparty == Thirdparty::gl)
			stbi_set_flip_vertically_on_load(true);

		pixels = stbi_load(filepath.c_str(), &w, &h, &channels, 0);
		LOG_IF(ERROR, pixels == nullptr) << "image failed to load at path: " << filepath;
		
		// const_cast<int&>(width)  = w;  const_cast<int&>(height) = h;
		manager.reset(pixels, [](unsigned char* pixels) { stbi_image_free(pixels); });
	}

	// 无需显式删除移动和拷贝函数？？
	// ~Image() { stbi_image_free(pixels); }

	int width()  const { return w; }
	int height() const { return h; }

	std::string file_path()  const { return filepath; }
	Thirdparty third_party() const { return thirdparty; }

	const unsigned char* data() const { return pixels; }

	GLenum gl_format() const
	{
		GLenum GLformat;

		switch (channels)
		{
		case 1: GLformat = GL_RED;	  break;	// grey
		case 2:	GLformat = GL_RG32UI; break;	// grey&alpha
		case 3: GLformat = GL_RGB;	  break;
		case 4: GLformat = GL_RGBA;	  break;
		default:
			LOG(ERROR) << "channels failed to choose" << filepath; break;
		}

		return GLformat;
	}

public:
	// const std::string filepath;
	// const Thirdparty thirdparty;
	// const int width{}, height{};

private:
	std::string filepath;
	Thirdparty thirdparty;

	unsigned char* pixels;
	int w, h, channels;
	std::shared_ptr<unsigned char> manager;
};

inline bool operator==(const Image& left, const Image& right)
{
	return left.file_path() == right.file_path();
}

} // namespace fay

#endif // FAY_UTILITY_IMAGE_H