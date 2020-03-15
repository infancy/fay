#include <gtest/gtest.h>

#include <fay/resource/file.h>
#include <fay/core/string.h>

using namespace std;
using namespace fay;

// -----------------------------------------------------------------------------

TEST(string, file)
{
	std::string filepath = "a/b/c.obj";

	ASSERT_EQ("a/b/",   get_directory(filepath));
	ASSERT_EQ("c.obj",  get_filename(filepath));
	ASSERT_EQ("obj",   get_filetype(filepath));
}


#pragma region regex

// a concept express
TEST(string, regex)
{
    // [a-zA-Z0-9_-]+@[a-zA-Z0-9_-]+(\.[a-zA-Z0-9_-]+)+
    regex_pattern email_0
    {
        "[a-zA-Z0-9_-]+"
        "@"
        "[a-zA-Z0-9_-]+"
        "(\.[a-zA-Z0-9_-]+)" 
        "+"
    };

    regex_pattern email_1
    {
        { "a-zA-Z0-9_-", "+" },
        { "@" },
        { "a-zA-Z0-9_-", "+" },
        { 
            { 
                { "\." },
                { "a-zA-Z0-9_-", "+" } 
            }, "+"
        }
    };

    regex_pattern email_2
        .match("a-zA-Z0-9_-", "+")
        .match("@")
        .match("a-zA-Z0-9_-", "+")
        .begin_group()
            .match("\.")
            .match("a-zA-Z0-9_-", "+")
        .end_group("+");

    regex_pattern phone_num_0
    {
        { "a", "|", "B" }
    };
}

#pragma endregion regex

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

// -------------------------------------------------------------------------------------------------

using strs = std::vector<std::string_view>;

TEST(string, split)
{
    {
        string str1{ "" };

        ASSERT_TRUE(split(str1).empty());
        ASSERT_TRUE(extracting_text(str1).empty());
    }

    {
        string str1{ " " };

        ASSERT_TRUE(split(str1).empty());
        ASSERT_TRUE(extracting_text(str1).empty());
    } 
    
    {
        string str1{ " >.> ^_^ " };
        strs ss1{ ">", ".", ">", "^", "_", "^" };
        strs ss2{ ".", "_" };

        ASSERT_EQ(split(str1), ss1);
        ASSERT_EQ(extracting_text(str1), ss2);
    }

    {
        string str1{ " (a_b, 1.f) " };
        strs ss1{ "(", "a_b", ",", "1.f", ")" };
        strs ss2{ "a_b", "1.f" };

        ASSERT_EQ(split(str1), ss1);
        ASSERT_EQ(extracting_text(str1), ss2);
    }

    {
        string str1{ "layout (location = 0) in vec3 mPos;" };
        strs ss1{ "layout", "(", "location", "=", "0", ")", "in", "vec3", "mPos", ";" };
        strs ss2{ "layout", "location", "0", "in", "vec3", "mPos" };

        ASSERT_EQ(split(str1), ss1);
        ASSERT_EQ(extracting_text(str1), ss2);
    }
}