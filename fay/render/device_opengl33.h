#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RENDER_DEVICE_OPENGL33_H
#define FAY_RENDER_DEVICE_OPENGL33_H

#include "glad/glad.h"

#include "fay/core/fay.h"
#include "fay/render/device.h"

namespace fay::render
{

class device_opengl33 : public render_device
{
	// glslVersion = getGLString(GL_SHADING_LANGUAGE_VERSION);
};

} // namespace fay

#endif // FAY_RENDER_DEVICE_OPENGL33_H