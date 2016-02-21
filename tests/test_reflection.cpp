#include "gtest/gtest.h"

#include <yato/reflection.h>


namespace
{
    YATO_REFLECTION_SET_COUNTER_VALUE(0);
    YATO_REFLECTION_GET_COUNTER_VALUE(test_val_1);

    YATO_REFLECTION_SET_COUNTER_VALUE(1);
    YATO_REFLECTION_GET_COUNTER_VALUE(test_val_2);

    YATO_REFLECTION_SET_COUNTER_VALUE(10);
    YATO_REFLECTION_GET_COUNTER_VALUE(test_val_3);
}

namespace
{
    struct A {};
    struct B {};
    struct C : public A {};

    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(A, 0);
    YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(A, typed_test_val_1);

    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(B, 10);
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(A, 1);
    YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(A, typed_test_val_2);

    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(A, 10);
    YATO_REFLECTION_SET_TYPED_COUNTER_VALUE(C, 110);
    YATO_REFLECTION_GET_TYPED_COUNTER_VALUE(A, typed_test_val_3);
}

TEST(Yato_Reflection, commons)
{

    static_assert(test_val_1 == 0,  "Reflection fail");
    static_assert(test_val_2 == 1,  "Reflection fail");
    static_assert(test_val_3 == 10, "Reflection fail");

    static_assert(typed_test_val_1 == 0, "Reflection fail");
    static_assert(typed_test_val_2 == 1, "Reflection fail");
    static_assert(typed_test_val_3 == 10, "Reflection fail");
}

namespace
{
    class Foo
    {
        YATO_REFLECT_CLASS(Foo)
    private:
        int x;
        YATO_REFLECT_VAR(x)
    public:
        float YATO_REFLECT_VAR_INLINE(y)
    };
}

#ifndef YATO_MSVC_2013
TEST(Yato_Reflection, trait)
{
    static_assert( yato::reflection::is_reflected<Foo>::value,  "reflection trait fail");
    static_assert(!yato::reflection::is_reflected<void>::value, "reflection trait fail");
}
#endif

TEST(Yato_Reflection, data_members)
{
    std::cout << typeid(Foo::_yato_reflected_y::my_class).name() << "::" << typeid(Foo::_yato_reflected_y::my_type).name() << std::endl;

    for (const auto & info : yato::reflection::reflection_manager<Foo>::instance()->members()) {
        std::cout << info.first << std::endl;
    }

    EXPECT_NE(nullptr, yato::reflection::reflection_manager<Foo>::instance()->get_by_name("x"));
    EXPECT_NE(nullptr, yato::reflection::reflection_manager<Foo>::instance()->get_by_name("y"));
    EXPECT_EQ(nullptr, yato::reflection::reflection_manager<Foo>::instance()->get_by_name("z"));

    Foo f;
    f.y = 0.0f;
    *static_cast<float*>(yato::reflection::reflection_manager<Foo>::instance()->get_by_name("y")->ptr(&f)) = 1.0f;
    // *static_cast<int*>(yato::reflection::reflection_manager<Foo>::instance()->get_by_name("x")->ptr(&f)) = 1; // <- should be error?
    EXPECT_EQ(1.0f, f.y);

    *(yato::reflection::reflection_manager<Foo>::instance()->get_by_name("y")->as_ptr<float>(&f)) = 2.0f;
    EXPECT_EQ(2.0f, f.y);

    using all_data_members = yato::reflection::details::reflection_manager_impl<Foo>::data_members_list;

    static_assert(std::is_same<all_data_members, yato::meta::list<
        yato::reflection::data_member_info<Foo, int>,
        yato::reflection::data_member_info<Foo, float> >
    >::value, "reflection fail!");
}

