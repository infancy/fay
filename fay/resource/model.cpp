#include "fay/resource/model.h"

namespace fay
{

resource_model::resource_model(const std::string& filepath, render_backend_type api)
    : filepath_{ filepath }
    , dir_{ get_directory(filepath_) }
    , api_{ api }
{
    if (auto str = get_filetype(filepath_); str == "obj")
        fmt_ = model_format::obj;
    else if (str == "gltf")
        fmt_ = model_format::gltf;
    else
        fmt_ = model_format::unknown;
}

std::string resource_model::directory() const { return dir_; }

model_format resource_model::format() const { return fmt_; }

bool resource_model::need_flip_image() const { return false; }



image convert_to_metallic_roughness(const std::string& directory, const std::string& ambient, const std::string& specular, bool flip_vertical)
{
    image ambi;
    image spec;
    image mr;

    if ((ambient.size() > 0) && (specular.size() > 0))
    {
        ambi = image(directory + ambient, flip_vertical);
        spec = image(directory + specular, flip_vertical);

        DCHECK((ambi.width() == spec.width()) && (ambi.height() == spec.height()));

        mr = image(ambi.width(), ambi.height(), pixel_format::rgb8, glm::u8vec4{ 0, 255, 0, 0 }); // pixel_format::rgba8 with ahpha = 0
    }
    else if (ambient.size() > 0)
    {
        ambi = image(directory + ambient, flip_vertical);
        mr = image(ambi.width(), ambi.height(), pixel_format::rgb8, glm::u8vec4{ 0, 255, 0, 0 });
    }
    else if (specular.size() > 0)
    {
        spec = image(directory + specular, flip_vertical);
        mr = image(spec.width(), spec.height(), pixel_format::rgb8, glm::u8vec4{ 0, 255, 0, 0 });
    }

    // maigc way to convert

    if (!ambi.empty())
    {
        auto src = ambi.data();
        auto dst = mr.data();
        for (uint32_t i = 0; i < mr.width() * mr.height(); ++i, dst += mr.channel(), src += ambi.channel())
            dst[2] = src[0]; // metallic, dst.b = src.r;
    }

    if (!spec.empty())
    {
        auto src = spec.data();
        auto dst = mr.data();
        for (uint32_t i = 0; i < mr.width() * mr.height(); ++i, dst += mr.channel(), src += spec.channel())
            dst[1] = src[0]; // roughness, dst.g = src.r;
    }

    return std::move(mr);
}


} // namespace fay