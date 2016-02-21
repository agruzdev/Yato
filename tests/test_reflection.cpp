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

TEST(Yato_Reflection, trait)
{
    static_assert( yato::reflection::is_reflected<Foo>::value,  "reflection trait fail");
    static_assert(!yato::reflection::is_reflected<void>::value, "reflection trait fail");
}

TEST(Yato_Reflection, members)
{
    std::cout << typeid(Foo::_yato_reflected_y::my_class).name() << "::" << typeid(Foo::_yato_reflected_y::my_type).name() << std::endl;

    for (const auto & info : yato::reflection::reflection_manager<Foo>::members()) {
        std::cout << info->name() << std::endl;
    }
}
