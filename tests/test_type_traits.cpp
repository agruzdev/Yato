#include "gtest/gtest.h"

#include <yato/type_traits.h>
#include <yato/range.h>

#include <vector>
#include <list>

TEST(Yato_TypeTraits, is_smart_ptr)
{
    using sptr = std::shared_ptr<int>;
    using uptr = std::unique_ptr<int>;

    EXPECT_TRUE(yato::is_shared_ptr<sptr>::value);
    EXPECT_TRUE(yato::is_unique_ptr<uptr>::value);
    
    EXPECT_FALSE(yato::is_shared_ptr<uptr>::value);
    EXPECT_FALSE(yato::is_unique_ptr<sptr>::value);
    
    EXPECT_FALSE(yato::is_unique_ptr<int>::value);
    EXPECT_FALSE(yato::is_unique_ptr<void>::value);
}

TEST(Yato_TypeTraits, is_iterator)
{
    EXPECT_TRUE(yato::is_iterator<std::vector<int>::iterator>::value);
    EXPECT_TRUE(yato::is_iterator<std::vector<int>::const_iterator>::value);

    EXPECT_TRUE(yato::is_iterator<std::vector<int>::reverse_iterator>::value);
    EXPECT_TRUE(yato::is_iterator<std::vector<int>::const_reverse_iterator>::value);

    EXPECT_TRUE(yato::is_iterator<std::list<float>::iterator>::value);
    EXPECT_TRUE(yato::is_iterator<std::list<float>::const_iterator>::value);

    EXPECT_FALSE(yato::is_iterator<std::vector<int>>::value);
    EXPECT_FALSE(yato::is_iterator<float>::value);
}

TEST(Yato_TypeTraits, numeric_iterator)
{
    EXPECT_TRUE(yato::is_iterator<yato::numeric_iterator<int>>::value);
    EXPECT_TRUE(yato::is_iterator<yato::numeric_iterator<size_t>>::value);
}

TEST(Yato_TypeTraits, is_same)
{
    class Foo {};
    class Bar : public Foo {};

    EXPECT_TRUE((true == yato::is_same<int, int>::value));
    EXPECT_TRUE((true == yato::is_same<short, short>::value));
    EXPECT_TRUE((true == yato::is_same<Foo, Foo, Foo, Foo, Foo, Foo, Foo>::value));
    EXPECT_FALSE((true == yato::is_same<int, float>::value));
    EXPECT_FALSE((true == yato::is_same<Foo, Bar>::value));
    EXPECT_FALSE((true == yato::is_same<Foo, const Foo>::value));
}

TEST(Yato_TypeTraits, has_trait)
{
    class Foo {};

    EXPECT_TRUE((true == yato::has_trait< std::is_integral, int >::value));
    EXPECT_TRUE((true == yato::has_trait< std::is_integral, int, long, short, bool >::value));
    EXPECT_FALSE((true == yato::has_trait< std::is_integral, int, long, Foo, bool >::value));
}

TEST(Yato_TypeTraits, one_of)
{
    class Foo {};

    EXPECT_TRUE((true == yato::one_of< int, int >::value));
    EXPECT_TRUE((true == yato::one_of< short, int, long, short, bool >::value));
    EXPECT_TRUE((true == yato::one_of< Foo, int, long, Foo, bool >::value));

    EXPECT_FALSE((true == yato::one_of< int, const int >::value));
    EXPECT_FALSE((true == yato::one_of< short, int, long, char, bool >::value));
    EXPECT_FALSE((true == yato::one_of< Foo, int, long, Foo*, bool >::value));
}

namespace
{
    class FooCallable
    {
    public:
        int operator()(){
            return 1;
        }
    };

    class FooNotCallable
    { };
}

#ifndef YATO_MSVC_2013
TEST(Yato_TypeTraits, is_callable)
{
    using t = decltype(&FooCallable::operator());
    static_assert(!yato::is_callable < int >::value, "is_callable fail");
    static_assert(!yato::is_callable < void >::value, "is_callable fail");
    static_assert(!yato::is_callable < FooNotCallable >::value, "is_callable fail");
    static_assert(yato::is_callable < FooCallable >::value, "is_callable fail");
    static_assert(yato::is_callable < std::function<void(void)> >::value, "is_callable fail");
}
#endif

namespace
{
    void foo(int, long);
    struct S
    {
        const char* bar();
    };
}

TEST(Yato_TypeTraits, functional)
{
    using foo_type = yato::function_pointer_to_type<decltype(&foo)>::type;
    static_assert(std::is_same<std::remove_pointer<decltype(&foo)>::type, void(int, long)>::value, "functional trait fail");
    static_assert(std::is_same<foo_type, std::function<void(int, long)>>::value, "functional trait fail");
    static_assert(std::is_same<foo_type::result_type, void>::value, "functional trait fail");

    using foo_trait = yato::function_trait<decltype(&foo)>;
    static_assert(std::is_same<foo_trait::result_type, void>::value, "functional trait fail");
    static_assert(foo_trait::arguments_num == 2, "functional trait fail");
    static_assert(std::is_same<foo_trait::arguments_list, yato::meta::list<int, long>>::value, "functional trait fail");
    static_assert(std::is_same<foo_trait::arg<0>::type, int>::value, "functional trait fail");
    static_assert(std::is_same<foo_trait::arg<1>::type, long>::value, "functional trait fail");

    using bar_trait = yato::function_member_trait<decltype(&S::bar)>;
    static_assert(std::is_same<bar_trait::result_type, const char*>::value, "functional trait fail");
    static_assert(bar_trait::arguments_num == 0, "functional trait fail");
    static_assert(std::is_same<bar_trait::arguments_list, yato::meta::null_list>::value, "functional trait fail");

    using bar_no_class = yato::remove_class<decltype(&S::bar)>::type;
    static_assert(std::is_same<bar_no_class, const char*(void)>::value, "functional trait fail");

    auto l = [](int, const float &)->double { return 0.0; };
    using l_type = decltype(l);

    using l_no_class = yato::remove_class<decltype(&l_type::operator())>::type;
    static_assert(std::is_same<l_no_class, double(int, const float &)>::value, "functional trait fail");

    using l_fun = yato::callable_to_function<decltype(l)>::type;
    static_assert(std::is_same<l_fun, std::function<double(int, const float &)>>::value, "functional trait fail");

    using l_traits = yato::callable_trait<decltype(l)>;
    static_assert(std::is_same<l_traits::result_type, double>::value, "functional trait fail");
    static_assert(l_traits::arguments_num == 2, "functional trait fail");
    static_assert(std::is_same<l_traits::arguments_list, yato::meta::list<int, const float&>>::value, "functional trait fail");

    using func = std::function<int(float, double)>;
    using func_trait = yato::callable_trait<func>;
    static_assert(std::is_same<func, func_trait::function_type>::value, "functional trait fail");
}
