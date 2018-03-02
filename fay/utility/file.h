#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_FILE_H
#define FAY_UTILITY_FILE_H

#include <string>

namespace fay
{

// retrieve the resources_directory path from the filepath
inline std::string get_path(const std::string& filepath)
{
	return filepath.substr(0, filepath.find_last_of('/')) + '/';
}

// retrieve the file name from the filepath
inline std::string get_name(const std::string& filepath)
{
	return filepath.substr(filepath.find_last_of('/') + 1);
}

inline std::string get_filetype(const std::string& filepath)
{
	return filepath.substr(filepath.find_last_of('.') + 1);
}

} // namespace fay

#endif // FAY_UTILITY_FILE_H
