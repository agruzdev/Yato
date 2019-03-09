/**
 * YATO library
 *
 * Apache License, Version 2.0
 * Copyright (c) 2016-2019 Alexey Gruzdev
 */

#include "gtest/gtest.h"

#include <atomic>
#include <memory>

#include <yato/variant.h>


TEST(Yato_Variant, details)
{
    using l1 = yato::meta::list<int, char, void, double>;
    using l2 = yato::meta::list<int*, void*>;
    using l3 = yato::meta::list<void>;
    using l4 = yato::meta::null_list;

    static_assert(yato::details::sizeof_ext<int>::value == sizeof(int), "yato::details::sizeof_ext fail");
    static_assert(yato::details::sizeof_ext<char>::value == sizeof(char), "yato::details::sizeof_ext fail");
    static_assert(yato::details::sizeof_ext<double>::value == sizeof(double), "yato::details::sizeof_ext fail");
    static_assert(yato::details::sizeof_ext<void>::value == 1, "yato::details::sizeof_ext fail");

    static_assert(yato::details::max_types_size<l1>::value == sizeof(double), "yato::details::max_types_size fail");
    static_assert(yato::details::max_types_size<l2>::value == sizeof(void*), "yato::details::max_types_size fail");
    static_assert(yato::details::max_types_size<l3>::value == 1, "yato::details::max_types_size fail");
    static_assert(yato::details::max_types_size<l4>::value == 1, "yato::details::max_types_size fail");

    static_assert(yato::details::max_types_alignment<l1>::value == std::alignment_of<double>::value, "yato::details::max_types_alignment fail");
    static_assert(yato::details::max_types_alignment<l2>::value == std::alignment_of<void*>::value, "yato::details::max_types_alignment fail");
    static_assert(yato::details::max_types_alignment<l3>::value == 1, "yato::details::max_types_alignment fail");
    static_assert(yato::details::max_types_alignment<l4>::value == 1, "yato::details::max_types_alignment fail");
}

TEST(Yato_Variant, common)
{
    yato::variant<void, int> v1;
    //yato::variant<char, int> v2;
    yato::variant<int, float> v3(1);
    yato::variant<int, float> v4(1.0f);
    //yato::variant<int, float> v4(1.0);

    EXPECT_EQ(typeid(void),  v1.type());
    EXPECT_EQ(typeid(int),   v3.type());
    EXPECT_EQ(typeid(float), v4.type());

    v3.emplace<void>();
    EXPECT_EQ(typeid(void), v3.type());
}

TEST(Yato_Variant, copy)
{
    //static_assert(yato::variant<void, int>::is_copy_constructible, "yato::variant<void, int> must be copyable");
    yato::variant<void, int> v1(1);
    yato::variant<void, int> v2 = yato::variant<void, int>(v1);

    yato::variant<float, void> v3;
    yato::variant<float, void> v4(v3);

    yato::variant<void, int> v5(std::move(v2));
    //EXPECT_EQ(typeid(void), v2.type());
    EXPECT_EQ(typeid(int), v5.type());

    yato::variant<void, int> v6(6);
    v1 = v6;
    yato::variant<void, int> v7;
    v5 = v7;

    yato::variant<void, int> v8;
    v8 = std::move(v1);
    EXPECT_NE(typeid(void), v1.type());
}

namespace
{
    struct CallsInfo
    {
        bool ctor = false;
        bool copyCtor = false;
        bool moveCtor = false;
        bool copyAssign = false;
        bool moveAssign = false;
        bool dtor = false;
    };

    class Foo
    {
        CallsInfo * info = nullptr;
    public:
        Foo(CallsInfo * infoRef)
            : info(infoRef)
        {
            if (info != nullptr) {
                info->ctor = true;
            }
        }

        Foo(const Foo & other)
        {
            if (other.info != nullptr) {
                other.info->copyCtor = true;
            }
        }

        Foo(Foo && other)
        {
            if (other.info != nullptr) {
                other.info->moveCtor = true;
            }
        }

        ~Foo()
        {
            if (info != nullptr) {
                info->dtor = true;
            }
        }

        Foo& operator = (const Foo & other)
        {
            if (other.info != nullptr) {
                other.info->copyAssign = true;
            }
            return *this;
        }

        Foo& operator = (Foo && other)
        {
            if (other.info != nullptr) {
                other.info->moveAssign = true;
            }
            return *this;
        }
    };
}

TEST(Yato_Variant, common2)
{
    yato::variant<void, int>  v1{ yato::in_place_type_t<void>() };
    yato::variant<int, float> v2(yato::in_place_type_t<int>(), 1);
    yato::variant<int, float> v3(yato::in_place_type_t<float>(), 1.0f);
    yato::variant<void, Foo>  v4(yato::in_place_type_t<Foo>(), nullptr);

    yato::variant<void, int>  v5{yato::in_place_index_t<0>() };
    yato::variant<int, float> v6(yato::in_place_index_t<0>(), 1);
    yato::variant<int, float> v7(yato::in_place_index_t<1>(), 1.0f);
    yato::variant<void, Foo>  v8(yato::in_place_index_t<1>(), nullptr);
}

TEST(Yato_Variant, copy2)
{
    CallsInfo info;
    yato::variant<void, Foo> v1;
    v1.emplace<Foo>(&info);

    EXPECT_TRUE(info.ctor);

    yato::variant<void, Foo> v2(v1);
    EXPECT_TRUE(info.copyCtor);

    yato::variant<void, Foo> v3(std::move(v1));
    EXPECT_TRUE(info.moveCtor);
    EXPECT_FALSE(info.dtor);
}

TEST(Yato_Variant, copy3)
{
    static_assert(std::is_copy_assignable<Foo>::value, "Foo should be CopyAssignable");
    static_assert(std::is_move_assignable<Foo>::value, "Foo should be MoveAssignable");

    CallsInfo info;
    yato::variant<void, Foo> v1;
    v1.emplace<Foo>(&info);

    EXPECT_TRUE(info.ctor);

    yato::variant<void, Foo> v2{ Foo(nullptr) };
    v2 = v1;
    EXPECT_TRUE(info.copyAssign);

    yato::variant<void, Foo> v3{ Foo(nullptr) };
    v3 = std::move(v1);
    EXPECT_TRUE(info.moveAssign);
    EXPECT_FALSE(info.dtor);
}

TEST(Yato_Variant, get)
{
    yato::variant<void, int> v1;
    yato::variant<int, float> v3(1);
    yato::variant<int, float> v4(1.0f);

    EXPECT_THROW(v1.get<1>(), yato::bad_variant_access);
    EXPECT_THROW(v3.get<1>(), yato::bad_variant_access);
    EXPECT_EQ(1, v3.get<0>());
    EXPECT_EQ(1.0f, v4.get<1>());

    EXPECT_EQ(2, v1.get<1>(2));
    EXPECT_EQ(3.0f, v3.get<1>(3.0f));
    EXPECT_EQ(1, v3.get<0>(1));
    EXPECT_EQ(1.0f, v4.get<1>(2.0f));
}

TEST(Yato_Variant, get2)
{
    yato::variant<void, int> v1;
    yato::variant<int, float> v3(1);
    yato::variant<int, float> v4(1.0f);

    EXPECT_THROW(v1.get_as<int>(), yato::bad_variant_access);
    EXPECT_THROW(v3.get_as<float>(), yato::bad_variant_access);
    EXPECT_EQ(1, v3.get_as<int>());
    EXPECT_EQ(1.0f, v4.get_as<float>());

    EXPECT_EQ(2, v1.get_as<int>(2));
    EXPECT_EQ(3.0f, v3.get_as<float>(3.0f));
    EXPECT_EQ(1, v3.get_as<int>(1));
    EXPECT_EQ(1.0f, v4.get_as<float>(2.0f));
}

TEST(Yato_Variant, atomic)
{
    yato::variant< void, std::atomic<char>, std::atomic<int> > v1;
    v1.emplace<std::atomic<int>>(2);
    EXPECT_EQ(2, v1.get<2>().load());
}

TEST(Yato_Variant, cast1)
{
    yato::variant<short, int, float> v1(42);
    yato::variant<short, char, int> v2('c');

    EXPECT_EQ(42,  v1.get_as<int>(0));
    EXPECT_EQ('c', v2.get_as<char>(0));

    v2 = yato::variant_cast<decltype(v2)::alternativies_list>(v1);

    EXPECT_EQ(42,  v2.get_as<int>(0));
}

TEST(Yato_Variant, cast2)
{
    yato::variant<void, int, float> v1(42);
    yato::variant<void, char, int> v2('c');

    EXPECT_EQ(42,  v1.get_as<int>(0));
    EXPECT_EQ('c', v2.get_as<char>(0));

    v2 = yato::variant_cast<decltype(v2)::alternativies_list>(v1);

    EXPECT_EQ(42,  v2.get_as<int>(0));
}

TEST(Yato_Variant, cast3)
{
    yato::variant<void, int, float> v1(42);
    yato::variant<short, char, int> v2('c');

    EXPECT_EQ(42,  v1.get_as<int>(0));
    EXPECT_EQ('c', v2.get_as<char>(0));

    v2 = yato::variant_cast<decltype(v2)::alternativies_list>(v1);

    EXPECT_EQ(42,  v2.get_as<int>(0));
}

TEST(Yato_Variant, cast4)
{
    yato::variant<short, int, float> v1(42);
    yato::variant<void, char, int> v2('c');

    EXPECT_EQ(42,  v1.get_as<int>(0));
    EXPECT_EQ('c', v2.get_as<char>(0));

    v2 = yato::variant_cast<decltype(v2)::alternativies_list>(v1);

    EXPECT_EQ(42,  v2.get_as<int>(0));
}

TEST(Yato_Variant, cast5)
{
    yato::variant<void, int, float> v1{};
    yato::variant<void, char, int> v2{};

    v2 = yato::variant_cast<decltype(v2)::alternativies_list>(v1);

    EXPECT_EQ(typeid(void), v2.type());
}

TEST(Yato_Variant, cast6)
{
    yato::variant<short, int, float> v1(1.0f);
    yato::variant<void, char, int> v2('c');

    EXPECT_EQ(1.0f,  v1.get_as<float>(0));
    EXPECT_EQ('c', v2.get_as<char>(0));

    EXPECT_THROW(v2 = yato::variant_cast<decltype(v2)::alternativies_list>(v1), yato::bad_variant_cast);
}

TEST(Yato_Variant, cast7)
{
    yato::variant<void, int, float> v1(42);
    yato::variant<short, char, int> v2('c');

    EXPECT_EQ(42,  v1.get_as<int>(0));
    EXPECT_EQ('c', v2.get_as<char>(0));

    v2 = yato::variant_cast<decltype(v2)::alternativies_list>(std::move(v1));

    EXPECT_EQ(42,  v2.get_as<int>(0));
}

TEST(Yato_Variant, non_copy)
{
    using ptr_variant =  yato::variant<std::unique_ptr<int>, std::unique_ptr<float>>;
    ptr_variant v1(std::make_unique<int>(42));
    ptr_variant v2(std::make_unique<int>(23));

    // v2 = v1; // no copy
    v2 = std::move(v1);

    std::vector<ptr_variant> vec;
    vec.push_back(std::move(v2));
    vec.emplace_back(std::make_unique<int>(7));
}

