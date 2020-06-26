#pragma once

#include <fstream>
#include <queue>
#include <sstream>

#include "fay/core/fay.h"


namespace fay
{

inline std::ifstream load_text(const std::string& filepath)
{
	std::ifstream file(filepath);
	CHECK(!file.fail()) << "can't open the file: " << filepath;
	return std::move(file);
}

inline std::string load_file_to_string(const std::string& filename)
{
	auto file = load_text(filename);

	std::stringstream str_stream{};
	str_stream << file.rdbuf();

	return str_stream.str();
}

// class file

// 二进制、文本，utf、GB213……
// template
class text
{
public:
	text();

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
