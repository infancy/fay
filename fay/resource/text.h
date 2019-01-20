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

// class file

// �����ơ��ı���utf��GB213����
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
	std::queue<std::string> line_cache;	// �û������и�����Ҫȷ�������������С���Ա� putline��
	// std::vector<std::string> line_cache;
};

} // namespace fay