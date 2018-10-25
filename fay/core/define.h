#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_DEFINE_H
#define FAY_CORE_DEFINE_H

namespace fay
{



// -------------------------------------------------------------------------------------------------
// graphics



// -------------------------------------------------------------------------------------------------
// math



enum class math_feature
{
	begin, none, simd, end
};



// -------------------------------------------------------------------------------------------------
// render



enum class render_backend
{
	none, opengl33, opengl45, d3d11, count
};
// TODO: FAY_ENUM_CLASS_OPERATOR( render_backend )


// -------------------------------------------------------------------------------------------------
// resource



enum class model_format
{
	begin, none, obj, fbx, gltf, blend, unknown, end
};

enum class texture_format
{
	begin, 
	none,
	diffuse, specular, ambient, emissive, height, alpha, parallax,
	normal, shininess, opacity, displace, lightmap, reflection, cubemap, shadowmap,
	unknown, 
	end
};



} // namespace fay

#endif // FAY_CORE_DEFINE_H
