#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/math/bounds.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

inline bool is_equal(glm::vec3 a, glm::vec3 b)
{
    return
        (is_equal(a.x, b.x)) &&
        (is_equal(a.y, b.y)) &&
        (is_equal(a.z, b.z));
}

inline bool is_equal(bounds3 a, bounds3 b)
{
    return
        (is_equal(a.min(), b.min())) &&
        (is_equal(a.max(), b.max()));
}

TEST(math, bounds3)
{
    bounds3 b3({ 1.f, -1.f, -1.f }, { -1.f, 1.f, 1.f });

    ASSERT_TRUE(is_equal(b3, bounds3(glm::vec3(1.f), glm::vec3(-1.f))));

    ASSERT_TRUE(is_equal(b3.center(), glm::vec3(0.f)));
    ASSERT_TRUE(is_equal(b3.coord(glm::vec3{ 2.f }), glm::vec3(3.f)));

    ASSERT_TRUE(is_equal(b3.radius(), std::sqrt(3.f)));

    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::I),    glm::vec3( 1.f,  1.f,  1.f)));
    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::II),   glm::vec3(-1.f,  1.f,  1.f)));
    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::III),  glm::vec3(-1.f, -1.f,  1.f)));
    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::IV),   glm::vec3( 1.f, -1.f,  1.f)));
    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::V),    glm::vec3( 1.f,  1.f, -1.f)));
    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::VI),   glm::vec3(-1.f,  1.f, -1.f)));
    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::VII),  glm::vec3(-1.f, -1.f, -1.f)));
    ASSERT_TRUE(is_equal(b3.corner(bounds_corner::VIII), glm::vec3( 1.f, -1.f, -1.f)));

    b3.expand(glm::vec3{ 2.f, -2.f, -2.f });
    b3.expand(glm::vec3{ -2.f, 2.f, 2.f });
    ASSERT_TRUE(is_equal(b3, bounds3(glm::vec3(2.f, 2.f, 2.f), glm::vec3(-2.f, -2.f, -2.f))));

    b3.expand(bounds3(glm::vec3(-3.f, 3.f, -3.f), glm::vec3(3.f, -3.f, 3.f)));
    ASSERT_TRUE(is_equal(b3, bounds3(glm::vec3(3.f, 3.f, 3.f), glm::vec3(-3.f, -3.f, -3.f))));

    // b3.transform
}