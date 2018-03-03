#include "gtest/gtest.h"

#include "fay/utility/file.h"
#include "fay/utility/string.h"

using namespace fay;

// -----------------------------------------------------------------------------

TEST(file, test1)
{
	std::string filepath = "a/b/c.obj";

	ASSERT_EQ("a/b/",   get_path(filepath));
	ASSERT_EQ("c.obj",  get_name(filepath));
	ASSERT_EQ("obj",   get_filetype(filepath));
}

// -----------------------------------------------------------------------------

TEST(string, erase_front_word)
{
	ASSERT_EQ("", erase_front_word(""));
	ASSERT_EQ("", erase_front_word("Hello"));
	ASSERT_EQ("", erase_front_word("Hello  "));
	ASSERT_EQ("", erase_front_word("  Hello"));
	ASSERT_EQ("", erase_front_word("  Hello  "));
	ASSERT_EQ("World", erase_front_word("Hello  World"));
	ASSERT_EQ("World  ", erase_front_word("  Hello  World  "));
}

TEST(string, erase_back_word)
{
	ASSERT_EQ("", erase_back_word(""));
	ASSERT_EQ("", erase_back_word("World"));
	ASSERT_EQ("", erase_back_word("World  "));
	ASSERT_EQ("", erase_back_word("  World"));
	ASSERT_EQ("", erase_back_word("  World  "));
	ASSERT_EQ("Hello", erase_back_word("Hello  World"));
	ASSERT_EQ("  Hello", erase_back_word("  Hello  World  "));
}

TEST(string, erase_white_spaces)
{
	ASSERT_EQ("", erase_white_spaces(""));
	ASSERT_EQ("", erase_white_spaces(" \t\f\v\n\r"));
	ASSERT_EQ("_", erase_white_spaces("_ \t\f\v\n\r"));
	ASSERT_EQ("_", erase_white_spaces(" \t\f\v\n\r_"));
	ASSERT_EQ("Hello World", erase_white_spaces("  Hello World  \n"));
}