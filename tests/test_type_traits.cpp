#include "gtest/gtest.h"

#include <yato/type_traits.h>
#include <yato/range.h>

#include <vector>
#include <list>

namespace
{
    class Foo
    {
    public:
        bool foo(int x) {
            return (x == 42);
        }

        void zoo() const {
        }
    };
}

TEST(Yato_TypeTraits, is_smart_ptr)
{
    using sptr = std::shared_ptr<Foo>;
    using uptr = std::unique_ptr<Foo>;
    using wptr = std::weak_ptr<Foo>;

    static_assert(yato::is_shared_ptr<sptr>::value, "is_smart_ptr fail");
    static_assert(yato::is_unique_ptr<uptr>::value, "is_smart_ptr fail");
    
    static_assert(!yato::is_shared_ptr<uptr>::value, "is_smart_ptr fail");
    static_assert(!yato::is_unique_ptr<sptr>::value, "is_smart_ptr fail");
    static_assert(!yato::is_shared_ptr<wptr>::value, "is_smart_ptr fail");
    static_assert(!yato::is_unique_ptr<wptr>::value, "is_smart_ptr fail");
    
    static_assert(!yato::is_unique_ptr<int>::value, "is_smart_ptr fail");
    static_assert(!yato::is_unique_ptr<void>::value, "is_smart_ptr fail");

    static_assert(yato::is_weak_ptr<wptr>::value, "is_smart_ptr fail");
    static_assert(!yato::is_weak_ptr<int>::value, "is_smart_ptr fail");
    static_assert(!yato::is_weak_ptr<void>::value, "is_smart_ptr fail");
    static_assert(!yato::is_weak_ptr<uptr>::value, "is_smart_ptr fail");
    static_assert(!yato::is_weak_ptr<sptr>::value, "is_smart_ptr fail");
}

TEST(Yato_TypeTraits, is_iterator)
{
    static_assert(yato::is_iterator<std::vector<int>::iterator>::value, "is_iterator fail");
    static_assert(yato::is_iterator<std::vector<int>::const_iterator>::value, "is_iterator fail");

    static_assert(yato::is_iterator<std::vector<int>::reverse_iterator>::value, "is_iterator fail");
    static_assert(yato::is_iterator<std::vector<int>::const_reverse_iterator>::value, "is_iterator fail");

    static_assert(yato::is_iterator<std::list<float>::iterator>::value, "is_iterator fail");
    static_assert(yato::is_iterator<std::list<float>::const_iterator>::value, "is_iterator fail");

    static_assert(!yato::is_iterator<std::vector<int>>::value, "is_iterator fail");
    static_assert(!yato::is_iterator<float>::value, "is_iterator fail");
}

TEST(Yato_TypeTraits, numeric_iterator)
{
    static_assert(yato::is_iterator<yato::numeric_iterator<int>>::value, "numeric_iterator fail");
    static_assert(yato::is_iterator<yato::numeric_iterator<size_t>>::value, "numeric_iterator fail");
}

TEST(Yato_TypeTraits, is_same)
{
    class Foo {};
    class Bar : public Foo {};

    static_assert((true == yato::is_same<int, int>::value), "is_same fail");
    static_assert((true == yato::is_same<short, short>::value), "is_same fail");
    static_assert((true == yato::is_same<Foo, Foo, Foo, Foo, Foo, Foo, Foo>::value), "is_same fail");
    static_assert(!(true == yato::is_same<int, float>::value), "is_same fail");
    static_assert(!(true == yato::is_same<Foo, Bar>::value), "is_same fail");
    static_assert(!(true == yato::is_same<Foo, const Foo>::value), "is_same fail");
}

TEST(Yato_TypeTraits, has_trait)
{
    class Foo {};

    static_assert((true == yato::has_trait< std::is_integral, int >::value), "has_trait fail");
    static_assert((true == yato::has_trait< std::is_integral, int, long, short, bool >::value), "has_trait fail");
    static_assert(!(true == yato::has_trait< std::is_integral, int, long, Foo, bool >::value), "has_trait fail");
}

TEST(Yato_TypeTraits, one_of)
{
    class Foo {};

    static_assert((true == yato::one_of< int, int >::value), "one_of fail");
    static_assert((true == yato::one_of< short, int, long, short, bool >::value), "one_of fail");
    static_assert((true == yato::one_of< Foo, int, long, Foo, bool >::value), "one_of fail");

    static_assert(!(true == yato::one_of< int, const int >::value), "one_of fail");
    static_assert(!(true == yato::one_of< short, int, long, char, bool >::value), "one_of fail");
    static_assert(!(true == yato::one_of< Foo, int, long, Foo*, bool >::value), "one_of fail");
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

TEST(Yato_TypeTraits, is_callable)
{
    auto lambda = [](int) -> float { return 1.0f; };

    using lambda_type = decltype(lambda);
    using op_type = decltype(&lambda_type::operator());

    static_assert(!yato::is_function_pointer<void>::value, "is_function_pointer fail");
    static_assert(!yato::is_function_pointer<int>::value, "is_function_pointer fail");
    static_assert(!yato::is_function_pointer<FooCallable>::value, "is_function_pointer fail");
    static_assert(!yato::is_function_pointer<FooNotCallable>::value, "is_function_pointer fail");
    static_assert(!yato::is_function_pointer<lambda_type>::value, "is_function_pointer fail");
    static_assert(!yato::is_function_pointer<std::function<int(void)>>::value, "is_function_pointer fail");

    static_assert(yato::is_function_pointer<int(*)(float)>::value, "is_function_pointer fail");
    static_assert(yato::is_function_pointer<op_type>::value, "is_function_pointer fail");
    static_assert(yato::is_function_pointer<decltype(&FooCallable::operator())>::value, "is_function_pointer fail");
    //-------------------------------------------------------

    static_assert(!yato::has_operator_round_brackets<void>::value, "has_operator_round_brackets fail");
    static_assert(!yato::has_operator_round_brackets<int>::value, "has_operator_round_brackets fail");
    static_assert(!yato::has_operator_round_brackets<FooNotCallable>::value, "has_operator_round_brackets fail");
    static_assert(!yato::has_operator_round_brackets<int(*)(float)>::value, "has_operator_round_brackets fail");
    static_assert(!yato::has_operator_round_brackets<op_type>::value, "has_operator_round_brackets fail");
    static_assert(!yato::has_operator_round_brackets<decltype(&FooCallable::operator())>::value, "has_operator_round_brackets fail");

    using _t = decltype(&FooCallable::operator());
    using _v = yato::test_type<decltype(&FooCallable::operator())>::type;
    static_assert(std::is_same<void, _v>::value, "");

    static_assert(yato::has_operator_round_brackets<FooCallable>::value, "has_operator_round_brackets fail");
    static_assert(yato::has_operator_round_brackets<lambda_type>::value, "has_operator_round_brackets fail");
    static_assert(yato::has_operator_round_brackets<std::function<int(void)>>::value, "has_operator_round_brackets fail");

    //-------------------------------------------------------
    static_assert(!yato::is_callable<void>::value, "is_callable fail");
    static_assert(!yato::is_callable<int>::value, "is_callable fail");
    static_assert(!yato::is_callable<FooNotCallable>::value, "is_callable fail");

    static_assert(yato::is_callable<FooCallable>::value, "is_callable fail");
    static_assert(yato::is_callable<lambda_type>::value, "is_callable fail");
    static_assert(yato::is_callable<std::function<int(void)>>::value, "is_callable fail");
    static_assert(yato::is_callable<int(*)(float)>::value, "is_callable fail");
    static_assert(yato::is_callable<op_type>::value, "is_callable fail");
    static_assert(yato::is_callable<decltype(&FooCallable::operator())>::value, "is_callable fail");
}

namespace
{
    void foo(int, long) {}
    struct Struct
    {
        const char* bar() { return nullptr; };
    };

    template <typename _T>
    struct TemplatedStruct
    {
        _T t;
        const char* bar() { return nullptr; };
        const char* c_bar() const { return nullptr; };
        const char* v_bar() volatile { return nullptr; };
        const char* cv_bar() volatile { return nullptr; };
    };
}

TEST(Yato_TypeTraits, remove_class)
{
    static_assert(std::is_same<yato::remove_class<decltype(&Struct::bar)>::type, const char*(*)(void)>::value, "remove_class fail");
    static_assert(std::is_same<std::remove_pointer<yato::remove_class<decltype(&Struct::bar)>::type>::type, const char*(void)>::value, "remove_class fail");
    static_assert(std::is_same<yato::remove_class<decltype(&foo)>::type, void(*)(int, long)>::value, "remove_class fail");

    static_assert(std::is_same<yato::remove_class<decltype(&TemplatedStruct<int>::bar)>::type, const char*(*)(void)>::value, "remove_class fail");
    static_assert(std::is_same<yato::remove_class<decltype(&TemplatedStruct<int>::c_bar)>::type, const char*(*)(void)>::value, "remove_class fail");
    static_assert(std::is_same<yato::remove_class<decltype(&TemplatedStruct<int>::v_bar)>::type, const char*(*)(void)>::value, "remove_class fail");
    static_assert(std::is_same<yato::remove_class<decltype(&TemplatedStruct<int>::cv_bar)>::type, const char*(*)(void)>::value, "remove_class fail");
}

TEST(Yato_TypeTraits, callable_trait)
{
    using foo_trait = yato::callable_trait<decltype(&foo)>;
    static_assert(std::is_same<foo_trait::result_type, void>::value, "callable_trait fail");
    static_assert(foo_trait::arguments_num == 2, "callable_trait fail");
    static_assert(std::is_same<foo_trait::arguments_list, yato::meta::list<int, long>>::value, "callable_trait fail");
    static_assert(std::is_same<foo_trait::function_type, std::function<void(int, long)> >::value, "callable_trait fail");

    using func = std::function<int(float, double)>;
    using func_trait = yato::callable_trait<func>;
    static_assert(std::is_same<func_trait::result_type, int>::value, "callable_trait fail");
    static_assert(func_trait::arguments_num == 2, "callable_trait fail");
    static_assert(std::is_same<func_trait::arguments_list, yato::meta::list<float, double>>::value, "callable_trait fail");
    static_assert(std::is_same<func_trait::function_type, std::function<int(float, double)> >::value, "callable_trait fail");

    auto l = [](int, const float &)->double { return 0.0; };
    using l_type = decltype(l);

    using l_trait = yato::callable_trait<decltype(l)>;
    static_assert(yato::has_operator_round_brackets<l_type>::value, "callable_trait fail");
    static_assert(std::is_same<l_trait::result_type, double>::value, "callable_trait fail");
    static_assert(l_trait::arguments_num == 2, "callable_trait fail");
    static_assert(std::is_same<l_trait::arguments_list, yato::meta::list<int, const float&>>::value, "callable_trait fail");
    static_assert(std::is_same<l_trait::function_type, std::function<double(int, const float &)> >::value, "callable_trait fail");
}

TEST(Yato_TypeTraits, make_function)
{
    const auto l = [](int, const float &)->double { return 42.0; };

    auto f1 = yato::make_function(&foo);
    auto f2 = yato::make_function(std::function<int(float, double)>());
    auto f3 = yato::make_function(l);

    float x;
    EXPECT_EQ(42.0, f3(0, x));
}

TEST(Yato_TypeTraits, functional)
{
    using foo_trait = yato::details::function_trait<decltype(&foo)>;
    static_assert(std::is_same<foo_trait::result_type, void>::value, "functional trait fail");
    static_assert(foo_trait::arguments_num == 2, "functional trait fail");
    static_assert(std::is_same<foo_trait::arguments_list, yato::meta::list<int, long>>::value, "functional trait fail");
    static_assert(std::is_same<foo_trait::arg<0>::type, int>::value, "functional trait fail");
    static_assert(std::is_same<foo_trait::arg<1>::type, long>::value, "functional trait fail");

    using bar_trait = yato::details::function_member_trait<decltype(&Struct::bar)>;
    static_assert(std::is_same<bar_trait::result_type, const char*>::value, "functional trait fail");
    static_assert(bar_trait::arguments_num == 0, "functional trait fail");
    static_assert(std::is_same<bar_trait::arguments_list, yato::meta::null_list>::value, "functional trait fail");
}

TEST(Yato_TypeTraits, functional_2)
{
    {
        auto f = [](int &) {};
        using f_trait = yato::callable_trait<decltype(f)>;
        static_assert(std::is_same<f_trait::arg<0>::type, int &>::value, "functional trait fail");
    }
    {
        auto f = [](int &&) {};
        using f_trait = yato::callable_trait<decltype(f)>;
        static_assert(std::is_same<f_trait::arg<0>::type, int &&>::value, "functional trait fail");
    }
}
