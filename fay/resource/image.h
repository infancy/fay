#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RESOURCE_IMAGE_H
#define FAY_RESOURCE_IMAGE_H

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include "fay/gl/gl.h"
#include "fay/math/math.h"

namespace fay
{

class base_image
{
public:
	base_image(const std::string& filepath, bool flip_vertical) :
		filepath{ filepath }, is_flip_vertical_{ flip_vertical }
	{
	}

	base_image(const std::string& filepath, int width, int height, int format, bool flip_vertical) :
		filepath{ filepath }, w{ width }, h{ height }, fmt{ format }, is_flip_vertical_{ flip_vertical }
	{
	}

	int width()  const { return w; }
	int height() const { return h; }

	std::string file_path() const { return filepath; }
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

// protected:
//     int& inner_width();
//     int& inner_height();
//     ...
// private:

protected:
	std::string filepath;
	int w{}, h{}, fmt{};

	bool is_flip_vertical_{ false };
};

// -----------------------------------------------------------------------------

class image_ptr : public base_image	// shared_image, stbImagePtr
{
public:
	image_ptr(const std::string& filepath = {}, bool flip_vertical = false) :
		base_image(filepath, flip_vertical)
	{
		LOG_IF(ERROR, filepath.empty()) << "image path is empty!";

		if (flip_vertical)
			stbi_set_flip_vertically_on_load(true);

		auto ptr = stbi_load(filepath.c_str(), &w, &h, &fmt, 0);
		LOG_IF(ERROR, ptr == nullptr) << "image failed to load at path: " << filepath;
		
		// const_cast<int&>(width)  = w;  const_cast<int&>(height) = h;
		manager.reset(ptr, [](uint8_t* p) { stbi_image_free(p); });
	}

	const uint8_t* data() const { return manager.get(); }

private:
	std::shared_ptr<uint8_t> manager;
};

inline bool operator==(const image_ptr& left, const image_ptr& right)
{
	return left.file_path() == right.file_path();
}

// -----------------------------------------------------------------------------
/*
class image : public base_image
{
public:
	image(const std::string& filepath = {}, bool flip_vertical = false) :
		base_image(filepath, flip_vertical)
	{
		LOG_IF(ERROR, filepath.empty()) << "image path is empty!";

		if (thirdparty == render_backend::gl)
		{
			is_flip_vertical_ = true;
			stbi_set_flip_vertically_on_load(true);
		}

		uint8_t* ptr = stbi_load(filepath.c_str(), &w, &h, &fmt, 0);
		LOG_IF(ERROR, ptr == nullptr) << "image failed to load at path: " << filepath;

		pixels = std::vector<uint8_t>(ptr, ptr + w * h * fmt);	// TODO

		stbi_image_free(ptr);
	}

	image(const std::string& filepath, int width , int height, int format, bool flip_vertical = false) :
		base_image(filepath, width, height, format, flip_vertical), pixels(w * h * fmt)
	{
		// pixels.clear();
		// memcpy(pixels.data(), 0, pixels.size());
	}

	uint8_t* data() { return pixels.data(); }

	// s 0.0 ~ 1.0, t: 0.0 ~ 1.0
	uint8_t& operator()(float s, float t)
	{
		s *= w; t *= h;
		return operator()((size_t)s, (size_t)t);
	}
	// s 0 ~ w, t: 0 ~ h
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

// TODO
inline bool operator==(const image& left, const image& right)
{
	return left.file_path() == right.file_path();
}
*/
// -----------------------------------------------------------------------------

// quality: 0 ~ 100
inline bool save_jpg(const std::string& filename, int width, int height, int comp,
	const uint8_t* pixel, int quality = 80)
{
	int result = stbi_write_jpg(filename.c_str(), 
		width, height, comp, pixel, quality);
	return result == 0 ? false : true;
}

inline bool save_ppm(const std::string& filename, int width, int height,
	const uint8_t* pixel /*bool flip_vertically = false*/)
{
	std::ofstream ppm(filename);
	CHECK(!ppm.fail()) << "can't create file";

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

inline bool save_pgm(const std::string& filename, int width, int height, 
	const uint8_t* pixel)
{
	// auto pgm = create_file(filename);
	std::ofstream pgm(filename);
	CHECK(!pgm.fail()) << "can't create file";

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

#endif // FAY_RESOURCE_IMAGE_H