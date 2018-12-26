#pragma once

#include <cctype>
#include <string>

#include "fay/core/utility.h" // TODO: algorithm

namespace fay
{

// -------------------------------------------------------------------------------------------------

// from_char, to_char, to_string

inline int to_int(const char* str)        { return std::atoi(str); }
inline int to_int(const std::string& str) { return std::stoi(str); }
inline int to_int(std::string_view str)   { return std::stoi(std::string{str}); }

// to_string(enum class e)

// -------------------------------------------------------------------------------------------------

// TODO: strings : string + vector<string_view>

// -------------------------------------------------------------------------------------------------

// return string_view(like return raw pointer)

constexpr inline std::string_view WhiteSpaces{ " \t\f\v\n\r" }; // WARNNING: the first char is ' ', TODO: add '\0'??? 
constexpr inline std::string_view SpecialChar{ "[]<>(){},:;"   "!~&|^+-*/%="   "`@#$?"   "\"\'\\"} ; // without '_', '.'

// e.g. "  Hello  World  " -> "World  "
inline std::string erase_front_word(
	const std::string& str, std::string_view split = WhiteSpaces)
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
inline std::string erase_back_word(
	const std::string& str, std::string_view split = WhiteSpaces)
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
inline std::string erase_white_spaces(
	const std::string& str, std::string_view white_spaces = WhiteSpaces)
{
	std::size_t begin = str.find_first_not_of(white_spaces);
	std::size_t end = str.find_last_not_of(white_spaces);

	if (begin == std::string::npos)
		return "";
	else
		return str.substr(begin, end - begin + 1);
}

// -------------------------------------------------------------------------------------------------

// e.g. "layout (location = 0) in vec3 mPos;" -> "layout", "(", "location", "=", "0", ")", "in", "vec3", "mPos", ";"
// e.g. " (a_b, 1.f) " -> "(", "a_b", ",", "1.f", ")"
inline std::vector<std::string_view> split(
    std::string_view str,
    bool save_special_char = true,
    std::string_view special_char = SpecialChar,
    std::string_view spilt_char = WhiteSpaces)
{
    std::string untext_char = std::string(special_char) + std::string(spilt_char);

    std::vector<std::string_view> strs;

    size_t current_index = str.find_first_not_of(spilt_char);

    for (; current_index < str.size();)
    {
        char ch = str[current_index];

        if (any_of(spilt_char, std::equal_to{}, ch))
        {
            ++current_index;
        }
        else if(any_of(special_char, std::equal_to{}, ch))
        {
            if(save_special_char)
                strs.push_back({ str.data() + current_index, 1 });

            ++current_index;
        }
        else // current char is a text char
        {
            auto next_index = str.find_first_of(untext_char, current_index);

            if (next_index != std::string::npos)
            {
                strs.emplace_back(str.data() + current_index, next_index - current_index);
            }
            else
            {
                strs.emplace_back(str.data() + current_index, str.size() - current_index);
            }
            current_index = next_index;
        }
    }
    return strs;
}

// e.g. "layout (location = 0) in vec3 mPos;" -> "layout", "location", "0", "in", "vec3", "mPos"
// WARNNING: include text and num
inline std::vector<std::string_view> extracting_text(std::string_view str)
{
    return split(str, false);
}

} // namespace fay