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

    using t7 = yato::meta::list_to_tuple<l7>::type;
    static_assert(std::is_same<t7, std::tuple<float, double>>::value, "yato::meta::list fail!");
}

TEST(Yato_Meta, list_find)
{
    using l1 = yato::meta::list<int, void, float, void>;

    static_assert(yato::meta::list_find<l1, int>::value == 0, "yato::meta::list_find fail");
    static_assert(yato::meta::list_find<l1, void>::value == 1, "yato::meta::list_find fail");
    static_assert(yato::meta::list_find<l1, float>::value == 2, "yato::meta::list_find fail");
    static_assert(yato::meta::list_find<l1, char>::value == yato::meta::list_npos, "yato::meta::list_find fail");

    static_assert(std::is_same< yato::meta::list_at<l1, yato::meta::list_find<l1, int>::value>::type, int >::value, "yato::meta::list_find fail");
    static_assert(std::is_same< yato::meta::list_at<l1, yato::meta::list_find<l1, void>::value>::type, void >::value, "yato::meta::list_find fail");
    static_assert(std::is_same< yato::meta::list_at<l1, yato::meta::list_find<l1, float>::value>::type, float >::value, "yato::meta::list_find fail");
}

TEST(Yato_Meta, list_length)
{
    using l1 = yato::meta::list<int, char, void, double>;
    using l2 = yato::meta::list<int*, void*>;
    using l3 = yato::meta::list<void>;
    using l4 = yato::meta::null_list;

    static_assert(yato::meta::list_length<l1>::value == 4, "yato::meta::list_length fail");
    static_assert(yato::meta::list_length<l2>::value == 2, "yato::meta::list_length fail");
    static_assert(yato::meta::list_length<l3>::value == 1, "yato::meta::list_length fail");
    static_assert(yato::meta::list_length<l4>::value == 0, "yato::meta::list_length fail");
}
