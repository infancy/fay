#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RESOURCE_IMAGE_H
#define FAY_RESOURCE_IMAGE_H

#include <fstream>

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include "fay/core/define.h"
#include "fay/gl/gl.h"
#include "fay/math/math.h"

namespace fay
{

// class base_image

class image_view
{
public:
    image_view(const std::string& filepath, bool flip_vertical = false) : // TODO: change interface
        filepath_{ filepath }, is_flip_vertical_{ flip_vertical }
    {
        LOG_IF(ERROR, filepath.empty()) << "image path is empty!";

        if (flip_vertical)
            stbi_set_flip_vertically_on_load(true);

        auto ptr = stbi_load(filepath.c_str(), &w_, &h_, &stride_, 0);
        LOG_IF(ERROR, ptr == nullptr) << "image failed to load at path: " << filepath;

        fmt_ = pixel_format_(stride_);
        // const_cast<int&>(width)  = w_;  const_cast<int&>(height) = h_;
        manager_.reset(ptr, [](uint8_t* p) { stbi_image_free(p); });
    }

    std::string_view filepath() const { return filepath_; }
    /*remove*/std::string file_path() const { return filepath_; }

	int width()  const { return w_; }
	int height() const { return h_; }
    int stride() const { return stride_; }
    pixel_format format() const { return fmt_; }
	bool is_flip_vertical() const { return is_flip_vertical_; }

    /*remove*/GLenum gl_format() const
	{
		GLenum GLformat{};

		switch (fmt_)
		{
		case pixel_format::r8:    GLformat = GL_RED;	break;	// grey
		case pixel_format::rg8:	  GLformat = GL_RG32UI; break;	// grey&alpha
		case pixel_format::rgb8:  GLformat = GL_RGB;	break;
		case pixel_format::rgba8: GLformat = GL_RGBA;	break;
		default:
			LOG(ERROR) << "format failed to choose" << filepath_; break;
		}

		return GLformat;
	}

    const uint8_t* data() const { return manager_.get(); }

private:
    pixel_format pixel_format_(int fmt) const
    {
        switch (fmt)
        {
            // TODO: let's true???
            case 1: return pixel_format::r8;	// grey
            case 2:	return pixel_format::rg8;	// grey&alpha
            case 3: return pixel_format::rgb8;
            case 4: return pixel_format::rgba8;
            default:
                LOG(ERROR) << "shouldn't be here" << filepath_;
                return pixel_format::none;
        }
    }

protected:
	std::string filepath_;

    int w_{}, h_{}, stride_{};
    pixel_format fmt_{};
	bool is_flip_vertical_{ false };

    std::shared_ptr<uint8_t> manager_;
};

inline bool operator==(const image_view& left, const image_view& right)
{
    return left.filepath() == right.filepath();
}

// -----------------------------------------------------------------------------

/*remove*/class image_ptr : public image_view	// shared_image, stbImagePtr
{
public:
    image_ptr() = default;
    using image_view::image_view;
};

/*remove*/inline bool operator==(const image_ptr& left, const image_ptr& right)
{
    return left.filepath() == right.filepath();
}

// -----------------------------------------------------------------------------

/*
template<size_t N>
class image
{
public:
	image(const image_view& iv)
	{

        w_ = iv.width();
        h_ = iv.height();
        stride_ = iv.stride();
        fmt_ = iv.format();
		pixels_ = std::vector<uint8_t>(iv.data(), iv.data() + w_ * h_ * stride_);
	}

	image(int width , int height, pixel_format format)
	{
		// pixels.clear();
		// memcpy(pixels.data(), 0, pixels.size());
        pixels_ = std::vector<uint8_t>(w_ * h_ * stride_);	// TODO
	}

    int width()  const { return w_; }
    int height() const { return h_; }
    pixel_format format() const { return fmt_; }

	uint8_t*       data()       { return pixels_.data(); }
    const uint8_t* data() const { return pixels_.data(); }

	// s 0.0 ~ 1.0, t: 0.0 ~ 1.0
	uint8_t& operator()(float s, float t)
	{
		s *= w_; t *= h_;
		return operator()((size_t)s, (size_t)t);
	}
	// s 0 ~ w_, t: 0 ~ h_
	uint8_t& operator()(size_t s, size_t t)
	{
		size_t x = s % w_, y = t % h_;
		if (x < 0) x += w_;
		if (y < 0) y += h_;
		return pixels_[y * w_ + x];
	}

    bool save(const std::string& savename)
    {
        save_jpg(savename, w_, h_, stride_, nullptr);
    }

private:
    uint32_t w_{}, h_{}, stride_;
    pixel_format fmt_{};
	std::vector<uint8_t> pixels_;
};

// TODO

inline bool operator==(const image& left, const image& right)
{
    return left.file_path() == right.file_path();
}
*/

// -----------------------------------------------------------------------------

// quality: 0 ~ 100
inline bool save_jpg(const std::string& filename, int width, int height, int format,
	const uint8_t* pixel, int quality = 80)
{
	int result = stbi_write_jpg(filename.c_str(), 
		width, height, format, pixel, quality);
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