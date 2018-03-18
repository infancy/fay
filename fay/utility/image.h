#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_IMAGE_H
#define FAY_UTILITY_IMAGE_H

#include "fay/gl/gl.h"
#include "fay/utility/math.h"
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

namespace fay
{

class Image	// shared_image, stbImage
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

	const uint8_t* data() const { return pixels; }

	uint8_t operator()(float s, float t) const
	{
		s *= w; t *= h;
		return operator()((size_t)s, (size_t)t);
	}
	uint8_t operator()(size_t s, size_t t) const
	{
		size_t x = s % w, y = t % h;
		if (x < 0) x += w;
		if (y < 0) y += h;
		return pixels[y * w + x];
	}

	GLenum gl_format() const
	{
		GLenum GLformat{};

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

	uint8_t* pixels;
	int w, h, channels;
	std::shared_ptr<unsigned char> manager;
};

inline bool operator==(const Image& left, const Image& right)
{
	return left.file_path() == right.file_path();
}

// -----------------------------------------------------------------------------

inline bool save_jpg(std::string filename, int width, int height, int comp,
	const uint8_t* pixel, int quality = 80)
{
	int r = stbi_write_jpg(filename.c_str(), 
		width, height, comp, pixel, quality);
	return r == 0 ? false : true;
}

inline bool save_ppm(std::string filename, int width, int height,
	const uint8_t* pixel /*bool flip_vertically = false*/)
{
	std::ofstream ppm(filename);
	CHECK(!ppm.fail()) << "\ncan't create file";

	ppm << "P3\n" << width << " " << height << "\n255\n";

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, pixel += 3)
		{
			ppm << clamp<int>(pixel[0], 0, 255) << " "
				<< clamp<int>(pixel[1], 0, 255) << " "
				<< clamp<int>(pixel[2], 0, 255) << " ";
		}
		ppm << std::endl;
	}

	return true;
}

inline bool save_pgm(std::string filename, int width, int height, 
	const uint8_t* pixel)
{
	std::ofstream pgm(filename);
	CHECK(!pgm.fail()) << "\ncan't create file";

	pgm << "P2\n" << width << " " << height << "\n255\n";

	//for (int y = height - 1; y >= 0; --y)
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++pixel)
			pgm << clamp<int>(pixel[0], 0, 255) << " ";
		pgm << std::endl;
	}

	return true;
}

} // namespace fay

#endif // FAY_UTILITY_IMAGE_H