#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef FAY_GL_GLUTILITY_H
#define FAY_GL_GLUTILITY_H

#include "glad/glad.h"
#include "fay/utility/fay.h"

namespace fay
{

inline void gl_check_errors() { CHECK(glGetError() == GL_NO_ERROR); }

} // namespace fay


#endif // FAY_GL_GLUTILITY_H

