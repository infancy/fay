#include "fay/resource/image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace fay
{

image::image(const std::string& filepath, bool flip_vertical)
{
    LOG_IF(ERROR, filepath.empty()) << "image path is empty!";

    name_ = get_filename(filepath);
    filepath_ = filepath;
    is_flip_vertical_ = flip_vertical;
    is_load_from_file_ = true;
    is_hdr_ = get_filetype(filepath) == "hdr";

    if (flip_vertical)
        stbi_set_flip_vertically_on_load(true);

    // TODO: color_space
    void* src{};
    if (is_hdr_)
    {
        src = stbi_loadf(filepath.c_str(), &width_, &height_, &channel_, 4);
    }
    else
    {
        src = stbi_load(filepath.c_str(), &width_, &height_, &channel_, 4);
    }
    LOG_IF(ERROR, src == nullptr) << "image failed to load at path: " << filepath;

    channel_ = 4; // always read 4-channel
    fmt_ = to_format(channel());

    resize(size(), pixel_size());
    memcpy(data(), src, size() * pixel_size());

    stbi_image_free(src);
}

// -------------------------------------------------------------------------------------------------
// inline image load_image(const std::string& filepath)

// quality: 1 ~ 100
bool save_image(const std::string& file_path_name, int width, int height, int channel, bool flip_vertically,
    const uint8_t* pixel, int quality)
{
    stbi_flip_vertically_on_write(flip_vertically ? 1 : 0);

    int result{};

    std::string filepath = file_path_name + ((channel == 4) ? ".png" : ".jpg");

    if(channel == 4) // need alpha channel
        result = stbi_write_png(filepath.c_str(),
            width, height, channel, pixel, 0);
    else
        result = stbi_write_jpg(filepath.c_str(),
            width, height, channel, pixel, quality);

    return result == 0 ? false : true;
}

bool save_ppm(const std::string& filename, int width, int height,
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

bool save_pgm(const std::string& filename, int width, int height,
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