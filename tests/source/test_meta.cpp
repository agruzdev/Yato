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

TEST(Yato_Meta, list_split)
{
    using l1 = yato::meta::list<int, char, void, double>;
    using l2 = yato::meta::list<void>;
    using l3 = yato::meta::null_list;

    using s1 = yato::meta::list_split<l1, 0>::type;
    static_assert(std::is_same<s1::first_type,  yato::meta::null_list>::value, "yato::meta::list_split fail");
    static_assert(std::is_same < s1::second_type, l1 > ::value, "yato::meta::list_split fail");

    using s2 = yato::meta::list_split<l1, 1>::type;
    static_assert(std::is_same<s2::first_type,  yato::meta::list<int>>::value, "yato::meta::list_split fail");
    static_assert(std::is_same<s2::second_type, yato::meta::list<char, void, double>>::value, "yato::meta::list_split fail");

    using s3 = yato::meta::list_split<l1, 2>::type;
    static_assert(std::is_same<s3::first_type, yato::meta::list<int, char>>::value, "yato::meta::list_split fail");
    static_assert(std::is_same<s3::second_type, yato::meta::list<void, double>>::value, "yato::meta::list_split fail");

    using s4 = yato::meta::list_split<l1, 3>::type;
    static_assert(std::is_same<s4::first_type, yato::meta::list<int, char, void>>::value, "yato::meta::list_split fail");
    static_assert(std::is_same<s4::second_type, yato::meta::list<double>>::value, "yato::meta::list_split fail");

    using s5 = yato::meta::list_split<l1, 4>::type;
    static_assert(std::is_same<s5::first_type, l1>::value, "yato::meta::list_split fail");
    static_assert(std::is_same<s5::second_type, yato::meta::null_list>::value, "yato::meta::list_split fail");


    using s6 = yato::meta::list_split<l2, 0>::type;
    static_assert(std::is_same<s6::first_type, yato::meta::null_list>::value, "yato::meta::list_split fail");
    static_assert(std::is_same<s6::second_type, l2>::value, "yato::meta::list_split fail");
    using s7 = yato::meta::list_split<l2, 1>::type;
    static_assert(std::is_same<s7::first_type, l2>::value, "yato::meta::list_split fail");
    static_assert(std::is_same<s7::second_type, yato::meta::null_list>::value, "yato::meta::list_split fail");


    using s8 = yato::meta::list_split<l3, 0>::type;
    static_assert(std::is_same<s8::first_type, yato::meta::null_list>::value, "yato::meta::list_split fail");
    static_assert(std::is_same<s8::first_type, yato::meta::null_list>::value, "yato::meta::list_split fail");
}


namespace 
{
    struct Foo128
    {
        char x[128];
    };
}

TEST(Yato_Meta, list_merge)
{
    static_assert(sizeof(Foo128) == 128, "yato::meta::list_merge fail");

    using t1 = yato::meta::list<void, int16_t, int64_t>;
    using t2 = yato::meta::list<int32_t, Foo128>;
    using t3 = yato::meta::list<int8_t, int16_t>;

    using m1 = yato::meta::list_merge<t1, t2, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<m1, yato::meta::list<void, int16_t, int32_t, int64_t, Foo128>>::value, "yato::meta::list_merge fail");

    using m2 = yato::meta::list_merge<t1, t3, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<m2, yato::meta::list<void, int8_t, int16_t, int16_t, int64_t>>::value, "yato::meta::list_merge fail");

    using m3 = yato::meta::list_merge<t2, t3, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<m3, yato::meta::list<int8_t, int16_t, int32_t, Foo128>>::value, "yato::meta::list_merge fail");

    using m4 = yato::meta::list_merge<t2, yato::meta::null_list, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<m4, t2>::value, "yato::meta::list_merge fail");

    using m5 = yato::meta::list_merge<t2, yato::meta::list<void>, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<m5, yato::meta::list<void, int32_t, Foo128>>::value, "yato::meta::list_merge fail");
}

namespace
{
    template <size_t Size_>
    struct Bar
    {
        char x[Size_];
    };
}

TEST(Yato_Meta, list_sort)
{
    using t1 = yato::meta::list<int64_t, Foo128, void, int16_t>;
    using t2 = yato::meta::list<int32_t>;
    using t3 = yato::meta::list<uint32_t, int8_t, int16_t>;
    using t4 = yato::meta::list<void>;
    using t5 = yato::meta::list<Bar<10>, Bar<1>, Bar<8>, void, Bar<16>, Bar<7>, Bar<64>, Bar<128>, Bar<2>, Bar<134>, Bar<42>>;

    using s1 = yato::meta::list_sort<t1, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<s1, yato::meta::list<void, int16_t, int64_t, Foo128>>::value, "yato::meta::list_sort fail");

    using s2 = yato::meta::list_sort<t2, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<s2, yato::meta::list<int32_t>>::value, "yato::meta::list_sort fail");

    using s3 = yato::meta::list_sort<t3, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<s3, yato::meta::list<int8_t, int16_t, uint32_t>>::value, "yato::meta::list_sort fail");

    using s4 = yato::meta::list_sort<t4, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<s4, t4>::value, "yato::meta::list_sort fail");

    using s5 = yato::meta::list_sort<t5, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<s5, yato::meta::list<void, Bar<1>, Bar<2>, Bar<7>, Bar<8>, Bar<10>, Bar<16>, Bar<42>, Bar<64>, Bar<128>, Bar<134>>>::value, "yato::meta::list_sort fail");

    using s6 = yato::meta::list_sort<yato::meta::null_list, yato::meta::type_less_sizeof>::type;
    static_assert(std::is_same<s6, yato::meta::null_list>::value, "yato::meta::list_sort fail");
}
