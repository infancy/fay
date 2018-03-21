#include "texture.h"
#include "fay/utility/image.h"

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

BaseTexture::BaseTexture(GLenum target, GLint filtering, GLint wrap) : 
	id_{}, target_{ target }
{
	glGenTextures(1, &id_);
	LOG_IF(WARNING, id_ == 0u) << "Fail to create texture!";
	//glActiveTexture(GL_TEXTURE0 + tex_unit);
	glBindTexture(target_, id_);	// 之后任何的纹理指令都作用于当前绑定的纹理

	glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, filtering);

	glTexParameteri(target_, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(target_, GL_TEXTURE_WRAP_T, wrap);
	if (target_ == GL_PROXY_TEXTURE_3D ||
		target_ == GL_TEXTURE_CUBE_MAP)
		glTexParameteri(target_, GL_TEXTURE_WRAP_R, wrap);
	/*
	如果选择了边缘过滤，还需要指定边缘颜色
	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	*/

	gl_check_errors();
}

// -----------------------------------------------------------------------------

Texture2D::Texture2D(const std::string& filepath, TexType textype, bool Mipmap) 
	: BaseTexture(GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT), texture_type{ textype }
{
	const ImagePtr img(filepath, Thirdparty::gl);
	w = img.width(), h = img.height();
	format_ = img.gl_format();

	create_texture2d(format_, img.width(), img.height(), format_, GL_UNSIGNED_BYTE, img.data(), Mipmap);
}

Texture2D::Texture2D(const ImagePtr& img, TexType textype, bool Mipmap)
	: BaseTexture(GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT), texture_type{ textype }, 
	w{ img.width() }, h{ img.height() }
{
	CHECK(img.third_party() == Thirdparty::gl) << "image thirdparty error";
	format_ = img.gl_format();

	create_texture2d(format_, img.width(), img.height(), format_, GL_UNSIGNED_BYTE, img.data(), Mipmap);
}

Texture2D::Texture2D(GLint internalFormat, GLsizei width, GLsizei height,
	GLenum format, GLenum type, unsigned char* pixels, bool Mipmap, TexType textype)
	: BaseTexture(GL_TEXTURE_2D, GL_LINEAR, GL_REPEAT), texture_type{ textype },
	w{ width }, h{ height }
{
	format_ = format;	// TODO:internalFormat

	CHECK(pixels != nullptr) << "pixels in Texture2D is nullptr";
	create_texture2d(internalFormat, width, height, format, type, pixels, Mipmap);
}

void Texture2D::create_texture2d(GLint internalFormat, GLsizei width, GLsizei height,
	GLenum format, GLenum type, const unsigned char* data, bool Mipmap)
{
	// https://stackoverflow.com/questions/34497195/difference-between-format-and-internalformat
	
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
		format, type, data);

	if(Mipmap)
	{ 
		glGenerateMipmap(GL_TEXTURE_2D);	// 为当前绑定的纹理自动生成所有需要的多级渐远纹理
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// 多级纹理只用于纹理被缩小的情况
	}
	gl_check_errors();
}

// -----------------------------------------------------------------------------

Texture2DArray::Texture2DArray(std::vector<std::string> material_names, std::string path) :
	BaseTexture(GL_TEXTURE_2D_ARRAY), material_nums{ material_names.size() }
{
	for(int i = 0; i < material_names.size(); ++i)
	{ 
		const ImagePtr img(path + material_names[i], Thirdparty::gl);	// 当前路径 + 文件名
		auto format = img.gl_format();

		// 首次先创建一个 2D 纹理数组
		if (i == 0)
		{
			format_ = format;
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, img.width(), img.height(), material_nums, 0, format, GL_UNSIGNED_BYTE, NULL);
		}
		CHECK(format_ == format) << "Texture2DArray: different format!";
		//modify the existing texture
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, img.width(), img.height(), 1, format, GL_UNSIGNED_BYTE, img.data());
	}
	gl_check_errors();
}


} // namespace fay