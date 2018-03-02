#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_GL_GL_H
#define FAY_GL_GL_H

#include "fay/utility/fay.h"
#include "glad/glad.h"

namespace fay
{

inline void gl_check_errors() { CHECK(glGetError() == GL_NO_ERROR); }

} // namespace fay


#endif // FAY_GL_GL_H

