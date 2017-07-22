#include "gtest/gtest.h"

#include <yato/actors/actor_path.h>


TEST(Yato_Actors, path)
{
    auto p1 = yato::actors::actor_path("system", yato::actors::actor_scope::user, "test1");
    yato::actors::path_elements elems;
    ASSERT_TRUE(p1.parce(elems));
    ASSERT_EQ(1, elems.names.size());
    EXPECT_EQ("system", elems.system_name);
    EXPECT_EQ(yato::actors::actor_scope::user, elems.scope);
    EXPECT_EQ("test1", elems.names[0]);
}

TEST(Yato_Actors, path2)
{
    auto p1 = yato::actors::actor_path("yato://someSystem/system/parent/child");
    yato::actors::path_elements elems;
    ASSERT_TRUE(p1.parce(elems));
    ASSERT_EQ(2, elems.names.size());
    EXPECT_EQ("someSystem", elems.system_name);
    EXPECT_EQ(yato::actors::actor_scope::system, elems.scope);
    EXPECT_EQ("parent", elems.names[0]);
    EXPECT_EQ("child",  elems.names[1]);
}

TEST(Yato_Actors, path3)
{
    auto p1 = yato::actors::actor_path("yato://someSystem/system/parent//child/");
    yato::actors::path_elements elems;
    ASSERT_TRUE(p1.parce(elems));
    ASSERT_EQ(2, elems.names.size());
    EXPECT_EQ("someSystem", elems.system_name);
    EXPECT_EQ(yato::actors::actor_scope::system, elems.scope);
    EXPECT_EQ("parent", elems.names[0]);
    EXPECT_EQ("child", elems.names[1]);
}

TEST(Yato_Actors, path4)
{
    auto p1 = yato::actors::actor_path("//someSystem/system/parent//child/");
    yato::actors::path_elements elems;
    ASSERT_FALSE(p1.parce(elems));

    auto p2 = yato::actors::actor_path("yato://someSystem/scoped/parent/child");
    ASSERT_TRUE(p2.parce(elems));
    EXPECT_EQ(yato::actors::actor_scope::unknown, elems.scope);

    auto p3 = yato::actors::actor_path("yato://someSystem");
    ASSERT_FALSE(p3.parce(elems));
}
