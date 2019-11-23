#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/resource/image.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

TEST(resource, image)
{
    image img("texture/awesomeface.png", true);
    ASSERT_EQ(img.width(), 512);
    ASSERT_EQ(img.height(), 512);
    ASSERT_EQ(img.channel(), 4);
    ASSERT_EQ(img.format(), pixel_format::rgba8);
    ASSERT_EQ(img.filepath(), "texture/awesomeface.png");
    ASSERT_EQ(img.is_flip_vertical(), true);
    ASSERT_EQ(img.is_load_from_file(), true);

    ASSERT_TRUE(img.save("texture/test_save_image"));
    ASSERT_TRUE(img.save("texture/test_save_image1", true));
}