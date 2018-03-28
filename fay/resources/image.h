#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RESOURCES_IMAGE_H
#define FAY_RESOURCES_IMAGE_H

#include "fay/gl/gl.h"
#include "fay/utility/math.h"
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

namespace fay
{

class base_image
{
public:
	base_image(const std::string& filepath = {}) :
		filepath{ filepath }
	{
	}

	base_image(const std::string& filepath, int width, int height, int format) :
		filepath{ filepath }, w{ width }, h{ height }, fmt{ format }
	{
	}

	int width()  const { return w; }
	int height() const { return h; }

	std::string file_path()  const { return filepath; }
	bool is_flip_vertical() const { return is_flip_vertical_; }

	GLenum gl_format() const
	{
		GLenum GLformat{};

		switch (fmt)
		{
		case 1: GLformat = GL_RED;	  break;	// grey
		case 2:	GLformat = GL_RG32UI; break;	// grey&alpha
		case 3: GLformat = GL_RGB;	  break;
		case 4: GLformat = GL_RGBA;	  break;
		default:
			LOG(ERROR) << "format failed to choose" << filepath; break;
		}

		return GLformat;
	}

protected:
	std::string filepath;
	bool is_flip_vertical_{ false };

	int w, h, fmt;
};

// -----------------------------------------------------------------------------

class image_ptr : public base_image	// shared_image, stbImagePtr
{
public:
	image_ptr(const std::string& filepath = {}, third_party thirdparty = third_party::none) :
		base_image(filepath)
	{
		LOG_IF(ERROR, filepath.empty()) << "image path is empty: ";

		if (thirdparty == third_party::gl)
		{
			is_flip_vertical_ = true;
			stbi_set_flip_vertically_on_load(true);
		}

		pixels = stbi_load(filepath.c_str(), &w, &h, &fmt, 0);
		LOG_IF(ERROR, pixels == nullptr) << "image failed to load at path: " << filepath;
		
		// const_cast<int&>(width)  = w;  const_cast<int&>(height) = h;
		manager.reset(pixels, [](unsigned char* pixels) { stbi_image_free(pixels); });
	}

	const uint8_t* data() const { return pixels; }

private:
	uint8_t* pixels;
	std::shared_ptr<unsigned char> manager;
};

inline bool operator==(const image_ptr& left, const image_ptr& right)
{
	return left.file_path() == right.file_path();
}

// -----------------------------------------------------------------------------

class image : public base_image
{
public:
	image(const std::string& filepath = {}, third_party thirdparty = third_party::none) :
		base_image(filepath)
	{
		LOG_IF(ERROR, filepath.empty()) << "image path is empty\n";
		if (thirdparty == third_party::gl)
		{
			is_flip_vertical_ = true;
			stbi_set_flip_vertically_on_load(true);
		}

		uint8_t* ptr = stbi_load(filepath.c_str(), &w, &h, &fmt, 0);
		LOG_IF(ERROR, ptr == nullptr) << "image failed to load at path: " << filepath;

		pixels = std::vector<uint8_t>(ptr, ptr + w * h * fmt);	// TODO

		stbi_image_free(ptr);
	}

	image(const std::string& filepath, int width , int height, int format, third_party thirdparty = third_party::none) :
		base_image(filepath, width, height, format), pixels(w * h * fmt)
	{
		pixels.clear();
	}

	uint8_t* data() { return pixels.data(); }

	uint8_t& operator()(float s, float t)
	{
		s *= w; t *= h;
		return operator()((size_t)s, (size_t)t);
	}
	uint8_t& operator()(size_t s, size_t t)
	{
		size_t x = s % w, y = t % h;
		if (x < 0) x += w;
		if (y < 0) y += h;
		return pixels[y * w + x];
	}

private:
	std::vector<uint8_t> pixels;
};

inline bool operator==(const image& left, const image& right)
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

#endif // FAY_RESOURCES_IMAGE_H