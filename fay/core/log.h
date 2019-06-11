#pragma once

#include "fay/core/config.h"

#ifdef FAY_DEBUG 
#define FAY_FILE_LINE(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#else  
#define FAY_FILE_LINE(format,...) 
#endif  

namespace fay
{



} // namespace fay
