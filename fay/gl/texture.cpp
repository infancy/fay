#include "texture.h"

namespace fay
{

/*

纹理过滤：
GL_NEAREST  Nearest Neighbor Filtering，邻近过滤，OpenGL默认的纹理过滤方式，选择中心点最接近纹理坐标的那个像素
GL_LINEAR   (Bi)linear Filtering，线性过滤，基于纹理坐标附近的纹理像素，计算出一个插值，近似出这些纹理像素之间的颜色。

环绕方式：
GL_REPEAT			对纹理的默认行为。重复纹理图像。
GL_MIRRORED_REPEAT	和GL_REPEAT一样，但每次重复图片是镜像放置的。
GL_CLAMP_TO_EDGE	纹理坐标会被约束在0到1之间，超出的部分会重复纹理坐标的边缘，产生一种边缘被拉伸的效果。
GL_CLAMP_TO_BORDER	超出的坐标为用户指定的边缘颜色。

*/

base_texture::base_texture(GLenum target, GLint filtering, GLint wrap) : 
	id_{}, target_{ target }
{
	glGenTextures(1, &id_);
	LOG_IF(WARNING, id_ == 0u) << "Fail to create texture!";
	//glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(target_, id_);	// 之后任何的纹理指令都作用于当前绑定的纹理

	if (target_ != GL_TEXTURE_2D_MULTISAMPLE)
	{
		glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, filtering);
		glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, filtering);

		glTexParameteri(target_, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(target_, GL_TEXTURE_WRAP_T, wrap);
	}

	if (target_ == GL_PROXY_TEXTURE_3D ||
		target_ == GL_TEXTURE_CUBE_MAP)
		glTexParameteri(target_, GL_TEXTURE_WRAP_R, wrap);
	
	// 如果选择了边缘过滤，还需要指定边缘颜色

	gl_check_errors();
}

void base_texture::set_border_color(std::array<GLfloat, 4> borderColor)
{
	glTexParameterfv(target_, GL_TEXTURE_BORDER_COLOR, borderColor.data());
}

// -----------------------------------------------------------------------------

texture2d::texture2d(const std::string& filepath, texture_format textype, bool Mipmap) 
	: base_texture(GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT), textype_{ textype }
{
	const image_ptr img(filepath, true);

	create(img.gl_format(), img.width(), img.height(), img.gl_format(), GL_UNSIGNED_BYTE, img.data(), Mipmap);
}

texture2d::texture2d(const image_ptr& img, texture_format textype, bool Mipmap)
	: base_texture(GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT), textype_{ textype }
{
	CHECK(img.is_flip_vertical() == true) << "image thirdparty error";
	// create(format_, img.width(), img.height(), format_, GL_UNSIGNED_BYTE, img.data(), Mipmap);
	create(img.gl_format(), img.width(), img.height(), img.gl_format(), GL_UNSIGNED_BYTE, img.data(), Mipmap);
}

texture2d::texture2d(GLint filtering, GLint wrap, texture_format textype)
	// : base_texture(GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT), texture_format{ textype }
	: base_texture(GL_TEXTURE_2D, filtering, wrap), textype_{ textype }
{
}

void texture2d::create(GLint internalFormat, GLsizei width, GLsizei height,
	GLenum format, GLenum type, const uint8_t* data, bool Mipmap)
{
	// https://stackoverflow.com/questions/34497195/difference-between-format-and-internalformat
	
	w = width; h = height; format_ = internalFormat;

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);

	if(Mipmap)
	{ 
		glGenerateMipmap(GL_TEXTURE_2D);	// 为当前绑定的纹理自动生成所有需要的多级渐远纹理
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// 多级纹理只用于纹理被缩小的情况
	}
	gl_check_errors();
}

// -----------------------------------------------------------------------------

texture2d_array::texture2d_array(std::vector<std::string> material_names, std::string path) :
	base_texture(GL_TEXTURE_2D_ARRAY), material_nums{ material_names.size() }
{
	for(int i = 0; i < material_names.size(); ++i)
	{ 
		const image_ptr img(path + material_names[i], true);	// 当前路径 + 文件名
		auto format = img.gl_format();

		// 首次先创建一个 2D 纹理数组
		if (i == 0)
		{
			format_ = format;
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, img.width(), img.height(), material_nums, 0, format, GL_UNSIGNED_BYTE, NULL);
		}
		CHECK(format_ == format) << "texture2d_array: different format!";
		//modify the existing texture
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, img.width(), img.height(), 1, format, GL_UNSIGNED_BYTE, img.data());
	}
	gl_check_errors();
}

} // namespace fay