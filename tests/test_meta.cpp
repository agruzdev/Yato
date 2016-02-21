#include "gtest/gtest.h"

#include <yato/meta.h>

namespace
{
    template<typename _List>
    struct print_list
    {
        static void print()
        {
            std::cout << typeid(typename _List::head).name() << ", ";
            print_list<typename _List::tail>::print();
        }
    };

    template<>
    struct print_list<yato::meta::null_list>
    {
        static void print()
        {
            std::cout << std::endl;
        }
    };
}

TEST(Yato_Meta, list)
{
    using l1 = yato::meta::list<int, void, float>;
    using l2 = yato::meta::list_push_front<double, l1>::type;
    using l3 = yato::meta::reverse_list<l2>::type;
    using l4 = yato::meta::list_push_back<l3, long>::type;

    static_assert(std::is_same<l2, yato::meta::list<double, int, void, float>>::value, "yato::meta::list fail!");
    static_assert(std::is_same<l3, yato::meta::list<float, void, int, double>>::value, "yato::meta::list fail!");
    static_assert(std::is_same<l4, yato::meta::list<float, void, int, double, long>>::value, "yato::meta::list fail!");

    using l5 = yato::meta::list_cat< yato::meta::list<int, short>, yato::meta::list<float, double> >::type;
    using l6 = yato::meta::list_cat< yato::meta::list<int, short>, yato::meta::null_list >::type;
    using l7 = yato::meta::list_cat< yato::meta::null_list, yato::meta::list<float, double> >::type;

    static_assert(std::is_same<l5, yato::meta::list<int, short, float, double>>::value, "yato::meta::list fail!");
    static_assert(std::is_same<l6, yato::meta::list<int, short>>::value, "yato::meta::list fail!");
    static_assert(std::is_same<l7, yato::meta::list<float, double>>::value, "yato::meta::list fail!");
}
