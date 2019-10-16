#pragma once

#include <fstream>

#include <glm/glm.hpp>

#include "fay/core/define.h"
#include "fay/math/math.h"
#include "fay/resource/file.h"

namespace fay
{

enum class color_space
{
    linear,
    sRGB
};

// WARNNING: use "user_directory/filename" is enough, don't add file type like ".png", ".jpg".
bool save_image(const std::string& file_path_name, int width, int height, int format, bool flip_vertically,
    const uint8_t* pixel, int quality = 80);

// class base_image
// class shared_image
class image_context
{
public:
    image_context() = default;

    uint32_t width()   const { return width_; }
    uint32_t height()  const { return height_; }
    uint32_t size()    const { return width_ * height_; } // WARNNING: pixel nums, not byte nums

    // TODO: remove
    uint32_t channel() const { return channel_; }

    size_t pixel_size() const { return channel_ * (is_hdr_ ? 4 : 1); }

    pixel_format format() const { return fmt_; }

    std::string_view name() const { return name_; }
    std::string_view filepath() const { return filepath_; }

    bool is_flip_vertical() const { return is_flip_vertical_; }

protected:
    int width_{}, height_{};

    int channel_{};
    pixel_format fmt_{};

    bool is_hdr_{};
    bool is_flip_vertical_{};
    // bool is_load_from_file_{};

    // TODO: class image : public resource;
    std::string name_{};
    std::string filepath_{};
};

class image_view;

// base_image
// big matrix
// template<typename pixel_format, size_t dimension>
class image : public image_context
{
public:
    image() = default;
    image(const image& that) = default;// = delete;
    image& operator=(const image& that) = default;// = delete;
    image(image&& temp) = default;
    image& operator=(image&& temp) = default;
    ~image() = default;

    //image(const image_view& iv) : image_context(iv) 

    image(size_t width, size_t height, pixel_format format, glm::u8vec4 initial_value)
    {
        width_ = width;
        height_ = height;
        fmt_ = format;
        channel_ = to_channel(format);

        resize(width * height, pixel_size(), initial_value);
    }

    // image(const uint8_t* data, size_t size, pixel_format format)

    image(const std::string& filepath, bool flip_vertical = false);

    // image clone()

    bool save(const std::string& save_path_name, bool flip_vertically = false)
    {
        return save_image(save_path_name, width(), height(), channel(), flip_vertically, data());
    }

    uint8_t*       data() { return pixels_.data(); }
    const uint8_t* data() const { return pixels_.data(); }

    /*
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
    */
    bool empty() const { return pixels_.empty(); }
    bool is_load_from_file() const { return is_load_from_file_; }

    glm::u8vec1& gray(size_t s, size_t t) { return *(reinterpret_cast<glm::u8vec1*>(pixel(s, t, 1))); }
    glm::u8vec3& rgb (size_t s, size_t t) { return *(reinterpret_cast<glm::u8vec3*>(pixel(s, t, 3))); }
    glm::u8vec4& rgba(size_t s, size_t t) { return *(reinterpret_cast<glm::u8vec4*>(pixel(s, t, 4))); }

    const glm::u8vec1& gray(size_t s, size_t t) const { return *(reinterpret_cast<const glm::u8vec1*>(pixel(s, t, 1))); }
    const glm::u8vec3& rgb (size_t s, size_t t) const { return *(reinterpret_cast<const glm::u8vec3*>(pixel(s, t, 3))); }
    const glm::u8vec4& rgba(size_t s, size_t t) const { return *(reinterpret_cast<const glm::u8vec4*>(pixel(s, t, 4))); }

private:
    size_t to_channel(pixel_format fmt) const
    {
        switch (fmt)
        {
            case pixel_format::r8:    return 1;	// grey
            //case pixel_format::rg8:	  GLformat = GL_RG32UI; break;	// grey&alpha
            case pixel_format::rgb8:  return 3;
            case pixel_format::rgba8: return 4;
            default:
                LOG(ERROR) << "format failed to choose"; 
                return 0;
        }
    }

    pixel_format to_format(size_t _channel) const
    {
        if (is_hdr_)
        {
            switch (_channel)
            {
                case 1: return pixel_format::r32f;
                //case 2: return pixel_format::rg32f;
                case 3: return pixel_format::rgb32f;
                case 4: return pixel_format::rgba32f;
                default:
                    LOG(ERROR) << "format failed to choose";
                    return pixel_format();
            }
        }
        else
        {
            switch (_channel)
            {
                case 1: return pixel_format::r8;
                case 2: return pixel_format::rg8;
                case 3: return pixel_format::rgb8;
                case 4: return pixel_format::rgba8;
                default:
                    LOG(ERROR) << "format failed to choose";
                    return pixel_format();
            }
        }
    }

    void resize(size_t size, size_t pixel_size, glm::u8vec4 initial_value = { 0, 0, 0, 0xff })
    {
        DCHECK((size > 0) && (pixel_size > 0));

        pixels_.reserve(size * pixel_size);
        pixels_.resize(size * pixel_size);

        // FIXME
        /*
        size_t flag{};
        for (auto& value : pixels_)
        {
            value = initial_value[flag];

            if (++flag == 4)
                flag = 0;
        }
        */
    }

    uint8_t* pixel(size_t x, size_t y, size_t num)
    {
        DCHECK(channel() == num);
        return &pixels_[(y * width() + x) * num];
    }

    const uint8_t* pixel(size_t x, size_t y, size_t num) const
    {
        DCHECK(channel() == num);
        return &pixels_[(y * width() + x) * num];
    }

private:
    std::vector<uint8_t> pixels_{};

    bool is_load_from_file_{};
};

FAY_SHARED_PTR(image)

/* This probably doesn't useful.
class image_view : public image_context // WARNNING: don't inherited from image_context
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

int width()  const { return w_; }
int height() const { return h_; }
int stride() const { return stride_; }
pixel_format format() const { return fmt_; }
bool is_flip_vertical() const { return is_flip_vertical_; }

GLenum gl_format() const
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
    const uint8_t* data;
    //std::shared_ptr<uint8_t> manager_;
};

inline bool operator==(const image_view& left, const image_view& right)
{
    return left.filepath() == right.filepath();
}
*/

} // namespace fay