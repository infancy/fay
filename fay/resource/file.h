#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_RESOURCE_FILE_H
#define FAY_RESOURCE_FILE_H

#include "fay/core/fay.h"

#include <queue>

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

std::ifstream load_file(const std::string& filepath)
{
	std::ifstream file(filepath);
	CHECK(!file.fail()) << "can't open the file: " << filepath;

	std::cout << "loading the file: " << filepath << '\n';
	return std::move(file);
}

// 二进制、文本，utf、GB213……
// template
class File
{
public:
	File();

	std::string getline();
	std::string putline();
	char getchar();
	void putchar();
	size_t current_line();
	size_t current_char();

private:
	std::queue<std::string> line_cache;	// 用户可自行根据需要确定缓存的行数大小（以备 putline）
	// std::vector<std::string> line_cache;
};

} // namespace fay

#endif // FAY_RESOURCE_FILE_H
