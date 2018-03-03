#if defined(_MSC_VER)
#pragma once
#endif

#ifndef FAY_UTILITY_STRING_H
#define FAY_UTILITY_STRING_H

#include <string>

namespace fay
{

// e.g. "  Hello  World  " -> "World  "
std::string erase_front_word(
	const std::string& str, std::string split = " \t\f\v\n\r")
{
	// stringstream ss(str); ss >> tmp; return ss.str();
	std::size_t begin = str.find_first_not_of(split);

	if (begin == std::string::npos)
		return "";

	begin = str.find_first_of(split, begin);

	if (begin == std::string::npos)
		return "";

	begin = str.find_first_not_of(split, begin);

	if (begin == std::string::npos)
		return "";

	return str.substr(begin);
}

// e.g. "  Hello  World  " -> "  Hello"
std::string erase_back_word(
	const std::string& str, std::string split = " \t\f\v\n\r")
{
	std::size_t end = str.find_last_not_of(split);

	// if (end == 0)
	if (end == std::string::npos)
		return "";

	end = str.find_last_of(split, end);

	if (end == std::string::npos)
		return "";

	end = str.find_last_not_of(split, end);

	if (end == std::string::npos)
		return "";

	return str.substr(0, end + 1);
}

// e.g. "  Hello World  \n" -> "Hello World"

// return end-begin+1??
std::string erase_white_spaces(
	const std::string& str, std::string whitespaces = " \t\f\v\n\r")
{
	std::size_t begin = str.find_first_not_of(whitespaces);
	std::size_t end = str.find_last_not_of(whitespaces);

	if (begin == std::string::npos)
		return "";
	else
		return str.substr(begin, end - begin + 1);
}

} // namespace fay

#endif // FAY_UTILITY_STRING_H