#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_CORE_LOG_H
#define FAY_CORE_LOG_H

#include "fay/core/config.h"

#ifdef FAY_DEBUG 
#define FAY_FILE_LINE(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#else  
#define FAY_FILE_LINE(format,...) 
#endif  

namespace fay
{



} // namespace fay

#endif // FAY_CORE_LOG_H
