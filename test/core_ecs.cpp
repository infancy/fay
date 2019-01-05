#include <gtest/gtest.h>

#include "fay/core/fay.h"
#include "fay/core/ecs.h"
#include "fay/core/range.h"

using namespace std;
using namespace fay;

// -------------------------------------------------------------------------------------------------

struct AComponent : component
{
    FAY_COMPONENT_TYPE_HASH(AComponent)

        AComponent(int v)
        : v(v)
    {
    }
    int v;
};

struct BComponent : component
{
    FAY_COMPONENT_TYPE_HASH(BComponent)

        BComponent(int v)
        : v(v)
    {
    }
    int v;
};

struct CComponent : component
{
    FAY_COMPONENT_TYPE_HASH(CComponent)

        CComponent(int v)
        : v(v)
    {
    }
    int v;
};


TEST(ecs, ecs)
{
    entity_pool pool;
    auto a = pool.create_entity();
    a->create_component<AComponent>(1);
    a->create_component<BComponent>(20);

    auto& group_ab = pool.get_component_group<AComponent, BComponent>();
    auto& group_ba = pool.get_component_group<BComponent, AComponent>();
    auto& group_bc = pool.get_component_group<BComponent, CComponent>();

    //a->create_component<AComponent>(40); // replacing the before value
    a->get_component<AComponent>()->v = 10;

    ASSERT_TRUE(group_ab.size() == 1 && group_ba.size() == 1);
    for (auto& e : group_ab)
        ASSERT_TRUE((get<0>(e)->v == 10) && (get<1>(e)->v == 20));
    for (auto& e : group_ba)
        ASSERT_TRUE((get<0>(e)->v == 20) && (get<1>(e)->v == 10));

    ASSERT_TRUE(group_bc.empty());

    auto eb = pool.create_entity();
    eb->create_component<AComponent>(11);
    eb->create_component<BComponent>(21);
    eb->create_component<CComponent>(31);

    ASSERT_TRUE(group_ab.size() == 2 && group_ba.size() == 2);
    std::abs(get<0>(group_ab[0])->v - get<0>(group_ab[1])->v) == 1;
    std::abs(get<1>(group_ab[0])->v - get<1>(group_ab[1])->v) == 1;

    ASSERT_TRUE(group_bc.size() == 1); 
    for (auto& e : group_bc)
        ASSERT_TRUE((get<0>(e)->v == 21) && (get<1>(e)->v == 31));
}