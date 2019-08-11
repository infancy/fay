#include <glog/logging.h>
#include <gtest/gtest.h>

GTEST_API_ int main(int argc, char **argv)
{
    google::InitGoogleLogging(argv[0]);

#define FAY_TEST_SOMEONE
#ifdef FAY_TEST_SOMEONE
    char filter[] = "--gtest_filter=math.*";
    argv[argc++] = filter;
#endif // FAY_TEST_SOMEONE

    //testing::GTEST_FLAG(filter) = "utility.*";
	testing::InitGoogleTest(&argc, argv);
	auto result = RUN_ALL_TESTS();
	getchar();
	return result;
}

/*
test_template.cpp:



#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

TEST(test_case, test_name)
{
    ASSERT_TRUE(expr);
}



*/