#pragma once

#include "fay/core/string.h"

namespace fay
{

// retrieve the resources_directory path from the filepath
inline std::string get_directory(const std::string& filepath)
{
    return filepath.substr(0, filepath.find_last_of('/')) + '/';
}

// retrieve the file name from the filepath
inline std::string get_filename(const std::string& filepath)
{
    return filepath.substr(filepath.find_last_of('/') + 1);
}

inline std::string get_filetype(const std::string& filepath)
{
    return filepath.substr(filepath.find_last_of('.') + 1);
}

} // namespace fay
